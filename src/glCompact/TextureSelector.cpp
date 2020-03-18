#include "glCompact/TextureSelector.hpp"
#include "glCompact/ToolsInternal.hpp"
#include <stdexcept>

using namespace std;

namespace glCompact {
    TextureSelector::TextureSelector() :
        texture    (0),
        mipmapLevel(0),
        layer      (-1)
    {
    }

    TextureSelector::TextureSelector(
        TextureInterface& texture,
        uint32_t          mipmapLevel
    ) :
        texture    (&texture),
        mipmapLevel(mipmapLevel),
        layer      (-1)
    {
        UNLIKELY_IF (!texture.id)
            throw runtime_error("Trying to use empty texture object for Frame!");
        UNLIKELY_IF (mipmapLevel > texture.mipmapCount - 1)
            throw out_of_range ("Trying to select texture mipmapLevel that does not exist!");
    }

    TextureSelector::TextureSelector(
        TextureInterface& texture,
        uint32_t          mipmapLevel,
        uint32_t          layer
    ) :
        texture    (&texture),
        mipmapLevel(mipmapLevel),
        layer      (layer)
    {
        UNLIKELY_IF (!texture.id)
            throw runtime_error   ("Trying to use empty texture object for Frame!");
        UNLIKELY_IF (mipmapLevel > texture.mipmapCount - 1)
            throw out_of_range    ("Trying to select texture mipmapLevel that does not exist!");
        UNLIKELY_IF (!texture.isLayered())
            throw invalid_argument("Trying to select layer of unlayered texture!");
        UNLIKELY_IF (layer       > texture.z)
            throw out_of_range    ("Trying to select texture layer that does not exist!");
    }
}
