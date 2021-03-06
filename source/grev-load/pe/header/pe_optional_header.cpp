#include "reinterpret_copy.hpp"

#include "pe_optional_header.hpp"

namespace grev
{
    pe_optional_header pe_optional_header::inspect(std::u8string_view* const data_view)
    {
        pe_optional_header optional_header { };

        constexpr std::size_t         pos_relative_entry_point_address{16};
        reinterpret_copy(&optional_header.relative_entry_point_address,
                    data_view->substr(pos_relative_entry_point_address));

        constexpr std::size_t         pos_base_address{28};
        reinterpret_copy(&optional_header.base_address,
                    data_view->substr(pos_base_address));

        constexpr std::size_t         pos_relative_exports_address{96};
        reinterpret_copy(&optional_header.relative_exports_address,
                    data_view->substr(pos_relative_exports_address));

        constexpr std::size_t         pos_relative_imports_address{104};
        reinterpret_copy(&optional_header.relative_imports_address,
                    data_view->substr(pos_relative_imports_address));

        constexpr std::size_t    size{224};
        data_view->remove_prefix(size);

        return optional_header;
    }
}
