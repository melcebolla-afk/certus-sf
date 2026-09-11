#!/bin/bash
#
# Repo: certus-sf/tools/despliegue_catalogs.bash
# Sincroniza EvidenceRoot catalogs de certus-sf con la misma flota que
# /home/mcebolla/evidence/tools/despliegue_evidence.bash (b1–b11, c1–c4, c6).
#
# Origen (repo local):       /home/mcebolla/certus-sf/catalogs/
# Destino en cada servidor:  /home/mcebolla/Engines/certus-sf/certus-sf/catalogs/
#
# Copia TODO el árbol catalogs/ (consensus, iccf, mate, theoretical, README, …).
#
# Variables opcionales:
#   MAX_PARALLEL               — despliegues simultáneos (por defecto 4)
#   HEARTBEAT_SEC              — segundos entre avisos “en curso” (0 = off; default 20)
#   CERTUS_CATALOGS_RSYNC_DELETE — si =1, rsync --delete (espejo; borra extras remotos)
#   CERTUS_CATALOGS_SRC / CERTUS_CATALOGS_REMOTE — override paths

export SSHPASS="colter8"
SRC="${CERTUS_CATALOGS_SRC:-/home/mcebolla/certus-sf/catalogs}"
REMOTE="${CERTUS_CATALOGS_REMOTE:-/home/mcebolla/Engines/certus-sf/certus-sf/catalogs}"
USER=mcebolla

MAX_PARALLEL="${MAX_PARALLEL:-4}"
HEARTBEAT_SEC="${HEARTBEAT_SEC:-20}"

RESULT_DIR=$(mktemp -d) || exit 1
LOCK="${RESULT_DIR}/lock"
ACTIVE_DIR="${RESULT_DIR}/active"
mkdir -p "$ACTIVE_DIR"
trap 'rm -rf "$RESULT_DIR"' EXIT

say() {
	(
		flock 9
		printf '%s\n' "$*"
	) 9>"$LOCK"
}

if [[ ! -d "$SRC" ]]; then
	echo "ERROR: no existe el origen: $SRC" >&2
	exit 1
fi

PAYLOAD_SIZE=$(du -sh "$SRC" 2>/dev/null | awk '{print $1}')
PAYLOAD_SIZE="${PAYLOAD_SIZE:-?}"

deploy_one() {
	local idx=$1 total=$2 server=$3
	local R="${USER}@${server}"
	local marker="${ACTIVE_DIR}/${server}"

	local -a rsync_opts=(-a --human-readable --info=stats2)
	if [[ "${CERTUS_CATALOGS_RSYNC_DELETE:-0}" == "1" ]]; then
		rsync_opts+=(--delete)
	fi
	if (( MAX_PARALLEL <= 1 )); then
		rsync_opts+=(--info=progress2,stats2)
	fi

	local t0 t1 dur
	t0=$(date +%s)
	echo "$t0" >"$marker"
	say "[${idx}/${total}]  ${server}  START  mkdir+rsync (~${PAYLOAD_SIZE})"

	if ! sshpass -e ssh -o BatchMode=no -o ConnectTimeout=15 \
		-o StrictHostKeyChecking=accept-new \
		"$R" "mkdir -p '${REMOTE}'"; then
		say "[${idx}/${total}]  ${server}  FALLO (mkdir/ssh)"
		echo "FALLO|0" >"${RESULT_DIR}/${server}"
		rm -f "$marker"
		return 1
	fi
	say "[${idx}/${total}]  ${server}  rsync …"

	local rlog="${RESULT_DIR}/${server}.rsync.log"
	if sshpass -e rsync "${rsync_opts[@]}" -e ssh "${SRC}/" "${R}:${REMOTE}/" \
		>"$rlog" 2>&1; then
		t1=$(date +%s)
		dur=$((t1 - t0))
		say "[${idx}/${total}]  ${server}  OK      ${dur}s"
		echo "OK|${dur}" >"${RESULT_DIR}/${server}"
		rm -f "$marker"
		return 0
	fi
	say "[${idx}/${total}]  ${server}  FALLO (ver ${rlog})"
	echo "FALLO|0" >"${RESULT_DIR}/${server}"
	rm -f "$marker"
	return 1
}

SERVERS=(b{1..11} c{1..4} c6)
N=${#SERVERS[@]}

echo
echo "========================================================================"
echo "  Despliegue certus-sf catalogs/  ->  ${N} servidores"
echo "  Origen:  ${SRC}/"
echo "  Destino: ${USER}@<servidor>:${REMOTE}/"
echo "  Payload: ~${PAYLOAD_SIZE}  (árbol completo)"
if [[ "${CERTUS_CATALOGS_RSYNC_DELETE:-0}" == "1" ]]; then
	echo "  Modo:    rsync --delete"
else
	echo "  Modo:    rsync -a (sin --delete)"
fi
echo "  Paralelo: hasta ${MAX_PARALLEL} a la vez"
echo "========================================================================"
echo

failed=0
pids=()
i=0
T0=$(date +%s)
for server in "${SERVERS[@]}"; do
	((++i))
	while (( $(jobs -rp 2>/dev/null | wc -l) >= MAX_PARALLEL )); do
		wait -n 2>/dev/null || wait
	done
	deploy_one "$i" "$N" "$server" &
	pids+=($!)
done

if (( HEARTBEAT_SEC > 0 )); then
	while true; do
		alive=0
		for pid in "${pids[@]}"; do
			if kill -0 "$pid" 2>/dev/null; then
				alive=1
				break
			fi
		done
		(( alive == 0 )) && break
		sleep "$HEARTBEAT_SEC"
		active=()
		for f in "$ACTIVE_DIR"/*; do
			[[ -f "$f" ]] || continue
			active+=("$(basename "$f")")
		done
		if (( ${#active[@]} > 0 )); then
			elapsed=$(( $(date +%s) - T0 ))
			say "… en curso (${elapsed}s): ${active[*]}"
		fi
	done
fi

for pid in "${pids[@]}"; do
	wait "$pid" || failed=$((failed + 1))
done
T1=$(date +%s)
TOTAL_SEC=$((T1 - T0))

echo
echo "--- Resumen (orden de la lista de despliegue) ---"
printf '%-8s  %-6s  %s\n' "Servidor" "Estado" "Segundos"
for server in "${SERVERS[@]}"; do
	IFS='|' read -r st sec <"${RESULT_DIR}/${server}" || true
	if [[ "$st" == "OK" ]]; then
		printf '%-8s  %-6s  %s\n' "$server" "OK" "$sec"
	else
		printf '%-8s  %-6s  %s\n' "$server" "FALLO" "-"
	fi
done
echo
echo "--- Totales ---"
if (( failed == 0 )); then
	echo "  Correctos: ${N}/${N}   Tiempo total (reloj): ${TOTAL_SEC}s"
else
	echo "  Fallidos:  ${failed}   Correctos: $((N - failed))/${N}   Tiempo total (reloj): ${TOTAL_SEC}s" >&2
	exit 1
fi
exit 0
