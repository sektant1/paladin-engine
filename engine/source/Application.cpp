#include "Application.h"

namespace ENG
{

void Application::SetNeedsToBeClosed(bool value)
{
    m_needsToBeClosed = value;
}

bool Application::NeedsToBeClosed() const
{
    return m_needsToBeClosed;
}

}  // namespace ENG
