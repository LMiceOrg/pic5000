#include "settingdialog.h"
#include "ui_settingdialog.h"

#include "udp_pic5000.h"
#include <QSettings>
#include <QDebug>

SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog),
    bready(false)
{
    ui->setupUi(this);
    char* devlist;
    int size;
    struct netdev_i
    {
        char name[128];
        char desc[128];
    };

    int ret = get_netdev_list(&devlist, &size);
    if(ret == 0)
    {
        qDebug()<<size;
        //ui->comboBox->disconnect(ui->comboBox, SIGNAL(currentIndexChanged(int)) );
        ui->comboBox->addItem(tr("No Selection"), "");
        for(int i=0; i< size; ++i)
        {
            struct netdev_i* dev = (struct netdev_i*)devlist + i;
            ui->comboBox->addItem(tr("%1 (%2) ").arg(dev->desc).arg(dev->name), dev->name);
        }
        //ui->comboBox->connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(on_comboBox_currentIndexChanged(int)) );
        free_netdev_list(&devlist);
    }

    QSettings setting("pic5000.ini", QSettings::IniFormat);
    QString net_name = setting.value("net/channel0", "en5").toString();


    for(int i=0; i<ui->comboBox->count(); ++i)
    {
        if(net_name.compare( ui->comboBox->itemData(i).toString()) == 0)
        {
            ui->comboBox->setCurrentIndex(i);
            break;
        }
    }
    bready = true;

}

SettingDialog::~SettingDialog()
{
    delete ui;
}

void SettingDialog::on_comboBox_currentIndexChanged(int index)
{
    if(!bready)
        return;
    QSettings setting("pic5000.ini", QSettings::IniFormat);
    setting.setValue("net/channel0", ui->comboBox->itemData(index).toString() );
}
