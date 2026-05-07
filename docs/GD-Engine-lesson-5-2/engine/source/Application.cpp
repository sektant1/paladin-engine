#include "Application.h"

namespace eng
{
    void Application::RegisterTypes()
    {
    }

    void Application::SetNeedsToBeClosed(bool value)
    {
        m_needsToBeClosed = value;
    }

    bool Application::NeedsToBeClosed() const
    {
        return m_needsToBeClosed;
    }
}