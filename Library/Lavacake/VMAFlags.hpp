#pragma once
#include "./Device.hpp"

namespace vk{


    /**
     * \brief cpp style wrapper for VMA AllocationCreateFlagBits
     */
    enum class AllocationCreateFlagBits : VkFlags
    {
    
        eCreateDedicatedMemory                                      = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        eCreateNeverAllocate                                        = VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT,
        eCreateMapped                                               = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        eCreateUserDataCopyString                                   = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT,
        eCreateUpperAddress                                         = VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT,
        eCreateDontBind                                             = VMA_ALLOCATION_CREATE_DONT_BIND_BIT,
        CreateWithinBudget                                          = VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT,
        eCreateCanAlias                                             = VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT,
        eCreateHostAccessSequentialWrite                            = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT ,
        eCreateHostAccessRandom                                     = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
        eCreateHostAccessAllowTransferInstead                       = VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT,
        eCreateStrategyMinMemory                                    = VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT,
        eCreateStrategyMinTime                                      = VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT,
        eCreateStrategyMinOffset                                    = VMA_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT,
        eCreateStrategyBestFit                                      = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT,
        eCreateStrategyFirstFit                                     = VMA_ALLOCATION_CREATE_STRATEGY_FIRST_FIT_BIT,
        eCreateStrategyMask                                         = VMA_ALLOCATION_CREATE_STRATEGY_MASK,
        eCreateFlagBitsMaxEnum                                      = VMA_ALLOCATION_CREATE_FLAG_BITS_MAX_ENUM
        
    };


    // here to allow the usage of AllocationCreateFlagBits similarly as vk flags
    using AllocationCreateFlags = Flags<AllocationCreateFlagBits>;


    // allow bitwise opperation on AllocationCreateFlagBits
    template <>
    struct FlagTraits<AllocationCreateFlagBits>
    {
        static constexpr bool isBitmask = true;
    };

    static_assert(
    std::is_same_v<
        std::underlying_type_t<vk::AllocationCreateFlagBits>,
        VkFlags>);


}

