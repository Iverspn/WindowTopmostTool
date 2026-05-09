#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <windows.h>
#include <tlhelp32.h>

struct ProcessInfo {
    QString name;
    DWORD pid;
    HWND hwnd;
    QString windowTitle;
};

class ProcessManager : public QObject
{
    Q_OBJECT

public:
    explicit ProcessManager(QObject *parent = nullptr);
    ~ProcessManager();

    QList<ProcessInfo> getProcessList();
    bool setWindowTopMost(HWND hwnd, bool topMost);
    bool isWindowTopMost(HWND hwnd);
    void startAutoRefresh(int intervalMs = 2000);
    void stopAutoRefresh();
    bool bringWindowToTop(HWND hwnd);

signals:
    void processListUpdated(const QList<ProcessInfo>& processes);

private:
    QTimer *refreshTimer;
    QList<ProcessInfo> cachedProcesses;

    static BOOL CALLBACK enumWindowsCallback(HWND hwnd, LPARAM lParam);
    QString getWindowTitle(HWND hwnd);
    DWORD getProcessId(HWND hwnd);
    QString getProcessName(DWORD pid);
    bool isMainWindow(HWND hwnd);
};

#endif // PROCESSMANAGER_H
