#pragma once


#ifdef IAENGINE_EXPORTS
#define IAENGINE_API __declspec(dllexport)
#else
#define IAENGINE_API __declspec(dllimport)
#endif

// ASSERT
#ifdef _DEBUG
#define IA_ASSERT( bTest ) { if ( !(bTest) ) { MessageBoxA( NULL, #bTest, "Assert", 0 ); __debugbreak(); } }
#else
#define IA_ASSERT( bTest )
#endif

//Safe delete and release
#define IA_SAFE_DELETE_ARRAY( pData ) { if (pData != NULL)		{				delete [] pData;				pData = NULL;		} }
#define IA_SAFE_RELEASE( pData ) { if (pData != NULL)		{				pData->Release();				pData = NULL;		} }

//Memory
#ifdef _DEBUG
#else
#endif

// warning for export dll for typedef of vetor<>
#pragma warning( disable : 4251 )

//Check Error
#define IA_TEST_OK( bTest ) { bool bRes = (bTest); if (! (bRes) ) return bRes; }
#define IA_CHECK_OK( iErrorCode ) { ErrorCode iErr = iErrorCode; if ( iErr != IA_OK ) { return iErr; } }
#define IA_CHECK_OK_NONE( iErrorCode ) { if ( iErrorCode != IA_OK ) {  } }
#define IA_CHECK_OK_RETURN( iErrorCode, bReturn ) { if ( iErrorCode != IA_OK ) { return bReturn; } }


//Singleton
#define IA_DECLARE_SINGLETON( TypeClass )														\
		private:																												\
			static TypeClass* s_pInstance;																\
		public:																													\
			static ErrorCode		CreateInstance();													\
			static void				DestroyInstance();													\
			static TypeClass*	GetInstance() { return s_pInstance; }
			


#define IA_IMPLEMENTE_SINGLETON( TypeClass )								\
	TypeClass* TypeClass::s_pInstance = NULL;									\
	ErrorCode	TypeClass::CreateInstance()											\
	{																													\
		if ( s_pInstance != NULL )															\
			return IA_ERR;																				\
		s_pInstance = new TypeClass();													\
		return s_pInstance->Create();														\
	}																													\
	void TypeClass::DestroyInstance()													\
	{																													\
		if ( s_pInstance != NULL )															\
		{																												\
			s_pInstance->Destroy();																\
			delete s_pInstance;																		\
			s_pInstance = NULL;																		\
		}																												\
	}