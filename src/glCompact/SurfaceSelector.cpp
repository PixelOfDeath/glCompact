#include "glCompact/SurfaceSelector.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;

namespace glCompact {
    SurfaceSelector::SurfaceSelector() :
        surface    (0),
        mipmapLevel(0),
        layer      (-1)
    {
    }

    SurfaceSelector::SurfaceSelector(
        SurfaceInterface& surface,
        uint32_t          mipmapLevel
    ) :
        surface    (&surface),
        mipmapLevel(mipmapLevel),
        layer      (-1)
    {
        UNLIKELY_IF (!surface.id)
            throw runtime_error("Trying to use empty surface object for Frame!");
        UNLIKELY_IF (mipmapLevel > surface.mipmapCount - 1)
            throw out_of_range ("Trying to select surface mipmapLevel that does not exist!");
    }

    SurfaceSelector::SurfaceSelector(
        SurfaceInterface& surface,
        uint32_t          mipmapLevel,
        uint32_t          layer
    ) :
        surface    (&surface),
        mipmapLevel(mipmapLevel),
        layer      (layer)
    {
        UNLIKELY_IF (!surface.id)
            throw runtime_error   ("Trying to use empty surface object for Frame!");
        UNLIKELY_IF (mipmapLevel > surface.mipmapCount - 1)
            throw out_of_range    ("Trying to select surface mipmapLevel that does not exist!");
        UNLIKELY_IF (!surface.isLayered())
            throw invalid_argument("Trying to select layer of unlayered surface!");
        UNLIKELY_IF (layer       > surface.size.z)
            throw out_of_range    ("Trying to select surface layer that does not exist!");
    }
}
