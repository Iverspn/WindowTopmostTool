#include <QApplication>
#include <QMessageBox>
#include <QIcon>
#include "mainwindow.h"
#include <windows.h>

bool isRunningAsAdmin()
{
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&ntAuthority, 2,
                                 SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS,
                                 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("窗口置顶管理器");

    app.setWindowIcon(QIcon(":/icon.png"));

    // 检查管理员权限
    if (!isRunningAsAdmin()) {
        QMessageBox::information(nullptr, "提示",
                                 "建议以管理员权限运行此程序，\n"
                                 "以便更好地管理其他进程的窗口。\n\n"
                                 "当前以普通用户权限运行，部分功能可能受限。");
    }

    MainWindow w;
    w.setWindowIcon(QIcon(":/icon.png"));
    w.show();

    return app.exec();
}
