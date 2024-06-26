#pragma once

#include <cstring>

#define O_STRING_DESTRUCTOR                0x87F228
#define O_STRING_CONSTRUCTOR_VOID          0x87F31C
#define O_STRING_CONSTRUCTOR_CHAR          0x87F46C
#define O_STRING_OPERATOR_ASSIGN_CHAR      0x87F388
#define O_STRING_OPERATOR_ASSIGN           0x87F318


namespace dice {
	namespace std {
		class string {
		public:
			~string()
			{
				(*(void(__thiscall**)(string*))O_STRING_DESTRUCTOR)(this);
			}

			string()
			{
				memset(this, 0, sizeof(*this));
				(*(void(__thiscall**)(string*))O_STRING_CONSTRUCTOR_VOID)(this);
			}

			string(const char* _str)
			{
				memset(this, 0, sizeof(*this));
				(*(void(__thiscall**)(string*, const char*))O_STRING_CONSTRUCTOR_CHAR)(this, _str);
			}

			string& operator=(const char* _str)
			{
				return (*(string & (__thiscall**)(string*, const char*))O_STRING_OPERATOR_ASSIGN_CHAR)(this, _str);
			}

			string& operator=(const string& _wstr)
			{
				return (*(string & (__thiscall**)(string*, const string&))O_STRING_OPERATOR_ASSIGN)(this, _wstr);
			}

			size_t size() const {
				return _Mysize;
			}

			const char* c_str() const
			{
				return (16 <= _Myres ? _Bx._Ptr : _Bx._Buf);
			}
		protected:
			unsigned char _Alval[4];
			union _Bxty
			{
				char _Buf[16];
				char* _Ptr;
			} _Bx;
			unsigned int _Mysize;
			unsigned int _Myres;
		};

	}
}
