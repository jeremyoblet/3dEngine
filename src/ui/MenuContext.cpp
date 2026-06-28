#include "MenuContext.h"

void MenuContext::addMenu(const QString& title, std::vector<MenuAction> actions)
{
    m_menus.push_back({ title, std::move(actions) });
}

const std::vector<MenuEntry>& MenuContext::menus() const
{
    return m_menus;
}
