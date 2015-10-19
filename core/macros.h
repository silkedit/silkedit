#pragma once

#define DEFAULT_COPY_AND_MOVE(TypeName)           \
  TypeName(const TypeName&) = default;            \
  TypeName(TypeName&&) = default;                 \
  TypeName& operator=(const TypeName&) = default; \
  TypeName& operator=(TypeName&&) = default;

#define DEFAULT_MOVE(TypeName)    \
  TypeName(TypeName&&) = default; \
  TypeName& operator=(TypeName&&) = default;

#define DISABLE_COPY_AND_MOVE(TypeName)          \
  TypeName(const TypeName&) = delete;            \
  TypeName(TypeName&&) = delete;                 \
  TypeName& operator=(const TypeName&) = delete; \
  TypeName& operator=(TypeName&&) = delete;

#define DISABLE_COPY(TypeName)        \
  TypeName(const TypeName&) = delete; \
  TypeName& operator=(const TypeName&) = delete;
