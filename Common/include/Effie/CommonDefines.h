#pragma once

#define ASSERT_TRUE(CHECK, MSG) \
do                              \
if (CHECK) {                   \
    LOG(ERROR) << MSG;          \
    exit(-1);                   \
}                               \
while (false)                   \

#define DESTROYER(IN, FUNC) [](IN) { FUNC; }

#define NO_COPY(ClassName) ClassName(const ClassName&) = delete; ClassName& operator=(const ClassName&) = delete;
#define GETTER(Type, Func, Field) inline Type Func() { return Field; }
#define CONST_GETTER(Type, Func, Field) inline const Type& Func() { return Field; }