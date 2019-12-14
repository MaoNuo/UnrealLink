#pragma once

#include "std/hash.h"
#include "Polymorphic.h"

#include "Containers/UnrealString.h"
#include "UniquePtr.h"


//region FString

namespace rd {
	template <>
	class Polymorphic<FString> {
	public:
		static FString read(SerializationCtx& ctx, Buffer& buffer);

		static void write(SerializationCtx& ctx, Buffer& buffer, FString const& value);
	};

    template <>
    class Polymorphic<Wrapper<FString>> {
    public:
        static void write(SerializationCtx& ctx, Buffer& buffer, Wrapper<FString> const& value);
    };

    template <>
    std::string to_string(FString const& val);

    template <>
    struct hash<FString> {
        size_t operator()(const FString& value) const noexcept;
    };

    // template <typename T>
    // std::string to_string(TArray<T> const& val);

    template <typename T>
    struct hash<TArray<T>> {
        size_t operator()(const TArray<T>& value) const noexcept;
    };

    template<typename T>
    Wrapper<T> ToRdWrapper(TUniquePtr<T> && Ptr) {
        Wrapper<T> Result;
        Result.reset(std::move(Ptr).Release());
        return Result;
    }
}

extern template class rd::Polymorphic<FString>;
extern template class rd::Polymorphic<rd::Wrapper<FString>>;
extern template struct rd::hash<FString>;
// extern template class rd::Polymorphic<TArray<FString>, void>;

//endregion
