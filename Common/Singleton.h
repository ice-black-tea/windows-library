#pragma once


//单例模式，为类添加GetInstance函数
#define DECLARE_SINGLETON(class_name) \
    private: \
        class CInstance \
        { \
        public: \
            CInstance() \
            { \
                m_pInstance = NULL; \
            } \
            ~CInstance() \
            { \
                if (m_pInstance != NULL)  \
                { \
                    delete m_pInstance; \
                    m_pInstance = NULL; \
                } \
            } \
            class_name* GetInstance() \
            { \
                return m_pInstance != NULL ? m_pInstance : (m_pInstance = new class_name); \
            } \
        private: \
            class_name *m_pInstance; \
        }; \
    public: \
        static class_name* GetInstance() \
        { \
            static CInstance Instance; \
            return Instance.GetInstance(); \
        };