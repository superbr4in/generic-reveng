#pragma once

#include <memory>
#include <set>

#include <revengine/address_space_segment.hpp>
#include <revengine/data_section.hpp>
#include <revengine/machine_architecture.hpp>

namespace rev
{
    class process
    {
        std::u8string data_;

    protected:

        explicit process(std::u8string data);

    public:

        virtual ~process();

        virtual machine_architecture architecture() const = 0;
        virtual std::uint64_t start_address() const = 0;

        data_section operator[](std::uint64_t address) const;

    protected:

        virtual std::set<address_space_segment, address_space_segment::exclusive_address_order> const&
            segments() const = 0; // TODO std::span<...>

        std::u8string_view data_view() const;
        std::size_t data_size() const;

    public:

        static std::unique_ptr<process> load(std::u8string data);
    };
}
