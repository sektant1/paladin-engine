#include "Application.h"

namespace COA
{

void Application::SetNeedsToBeClosed(bool value)
{
    m_needsToBeClosed = value;
}

bool Application::NeedsToBeClosed() const
{
    return m_needsToBeClosed;
}

}  // namespace COA
