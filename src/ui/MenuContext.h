#pragma once
#include <functional>
#include <vector>
#include <QString>

struct MenuAction {
    QString               label;
    std::function<void()> trigger;
    bool                  separator = false;

    static MenuAction Separator() { return { {}, nullptr, true }; }
};

struct MenuEntry {
    QString                  title;
    std::vector<MenuAction>  actions;
};

class MenuContext {
public:
    void addMenu(const QString& title, std::vector<MenuAction> actions);
    const std::vector<MenuEntry>& menus() const;

private:
    std::vector<MenuEntry> m_menus;
};
