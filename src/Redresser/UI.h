#pragma once

namespace UI {
    void Register();

    namespace Settings {
        void __stdcall Render();
        void InitBuffers();
    }

    namespace Filters {
        void __stdcall Render();
    }
}
