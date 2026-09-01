/*
  certus-sf — thin integration layer (merge-friendly).
  All product logic lives in ../evidence/; upstream SF files touch this only.
*/

#ifndef CERTUS_ENGINE_H_INCLUDED
#define CERTUS_ENGINE_H_INCLUDED

#include <functional>

#include "../evidence/evidence_manager.h"
#include "../ucioption.h"

namespace Stockfish::Certus {

class EngineExtension {
   public:
    void register_options(OptionsMap& options, std::function<void()> on_reload) {
        manager_.register_options(options, std::move(on_reload));
    }

    Evidence::Manager&       evidence() { return manager_; }
    const Evidence::Manager& evidence() const { return manager_; }

   private:
    Evidence::Manager manager_;
};

std::string engine_identity(bool to_uci);

}  // namespace Stockfish::Certus

#endif
