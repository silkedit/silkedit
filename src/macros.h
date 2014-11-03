#pragma once

#define DISABLE_COPY_AND_MOVE(TypeName) \
  TypeName(const TypeName&) = delete;   \
  TypeName(TypeName&&) = delete;   \
  TypeName& operator=(const TypeName&) = delete; \
  TypeName& operator=(TypeName&&) = delete;

#define DISABLE_COPY(TypeName) \
  TypeName(const TypeName&) = delete;   \
  TypeName& operator=(const TypeName&) = delete; \
