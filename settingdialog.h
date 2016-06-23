#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class SettingDialog;
}

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = 0);
    ~SettingDialog();

private slots:
    void on_comboBox_currentIndexChanged(int index);

private:
    Ui::SettingDialog *ui;
    bool bready;
};

#endif // SETTINGDIALOG_H
