
#if defined(_MSC_VER)

#pragma warning(push)
#pragma warning(disable: 4127) // conditional expression is constant
#pragma warning(disable: 4251) // struct needs to have dll-interface to be used by clients of struct
#pragma warning(disable: 4275) // non dll-interface struct ... used as base for dll-interface struct
#pragma warning(disable: 4355) // this used in base member initializer list
#pragma warning(disable: 4800) // forcing value to bool 'true' or 'false'
#pragma warning(disable: 4311)

#define vsnprintf _vsnprintf
#define  snprintf  _snprintf

#endif // _MSC_VER
