/** \file factory.cpp
\brief Define the factory to create new instance
\author alpha_one_x86
\version 0.3
\date 2010 */

#include <QFileDialog>
#include <QDebug>

#include "Factory.h"

CopyEngineFactory::CopyEngineFactory() :
    ui(new Ui::copyEngineOptions())
{
    qRegisterMetaType<FolderExistsAction>("FolderExistsAction");
    qRegisterMetaType<FileExistsAction>("FileExistsAction");
    qRegisterMetaType<QList<Filters_rules> >("QList<Filters_rules>");
    qRegisterMetaType<TransferStat>("TransferStat");

    tempWidget=new QWidget();
    ui->setupUi(tempWidget);
    ui->blockSize->setMaximum(ULTRACOPIER_PLUGIN_MAX_BLOCK_SIZE);
    errorFound=false;
    optionsEngine=NULL;
    filters=new Filters(tempWidget);
    renamingRules=new RenamingRules(tempWidget);

    QStringList temp=storageInfo.allLogicalDrives();
    for (int i = 0; i < temp.size(); ++i) {
        mountSysPoint<<QDir::toNativeSeparators(temp.at(i));
    }
    connect(&storageInfo,&QStorageInfo::logicalDriveChanged,this,&CopyEngineFactory::logicalDriveChanged);

    connect(ui->doRightTransfer,		&QCheckBox::toggled,		this,&CopyEngineFactory::setDoRightTransfer);
    connect(ui->keepDate,			&QCheckBox::toggled,		this,&CopyEngineFactory::setKeepDate);
    connect(ui->blockSize,			static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),	this,&CopyEngineFactory::setBlockSize);
    connect(ui->autoStart,			&QCheckBox::toggled,		this,&CopyEngineFactory::setAutoStart);
    connect(ui->doChecksum,			&QCheckBox::toggled,		this,&CopyEngineFactory::doChecksum_toggled);
    connect(ui->comboBoxFolderError,	static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),		this,&CopyEngineFactory::setFolderError);
    connect(ui->comboBoxFolderCollision,	static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),		this,&CopyEngineFactory::setFolderCollision);
    connect(ui->comboBoxFileError,	static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),		this,&CopyEngineFactory::setFileError);
    connect(ui->comboBoxFileCollision,	static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),		this,&CopyEngineFactory::setFileCollision);
    connect(ui->checkBoxDestinationFolderExists,	&QCheckBox::toggled,		this,&CopyEngineFactory::setCheckDestinationFolder);
    connect(ui->checksumIgnoreIfImpossible,	&QCheckBox::toggled,		this,&CopyEngineFactory::checksumIgnoreIfImpossible_toggled);
    connect(ui->checksumOnlyOnError,	&QCheckBox::toggled,		this,&CopyEngineFactory::checksumOnlyOnError_toggled);
    connect(ui->osBuffer,			&QCheckBox::toggled,		this,&CopyEngineFactory::osBuffer_toggled);
    connect(ui->osBufferLimited,		&QCheckBox::toggled,		this,&CopyEngineFactory::osBufferLimited_toggled);
    connect(ui->osBufferLimit,		&QSpinBox::editingFinished,	this,&CopyEngineFactory::osBufferLimit_editingFinished);
    connect(ui->osBufferLimited,&QAbstractButton::toggled,this,&CopyEngineFactory::updateBufferCheckbox);
    connect(ui->osBuffer,&QAbstractButton::toggled,this,&CopyEngineFactory::updateBufferCheckbox);

    connect(filters,&Filters::sendNewFilters,this,&CopyEngineFactory::sendNewFilters);
    connect(ui->filters,&QPushButton::clicked,this,&CopyEngineFactory::showFilterDialog);
    connect(renamingRules,&RenamingRules::sendNewRenamingRules,this,&CopyEngineFactory::sendNewRenamingRules);
    connect(ui->renamingRules,&QPushButton::clicked,this,&CopyEngineFactory::showRenamingRules);

    lunchInitFunction.setInterval(0);
    lunchInitFunction.setSingleShot(true);
    connect(&lunchInitFunction,&QTimer::timeout,this,&CopyEngineFactory::init,Qt::QueuedConnection);
    lunchInitFunction.start();
}

CopyEngineFactory::~CopyEngineFactory()
{
    delete renamingRules;
    delete filters;
    delete ui;
}

void CopyEngineFactory::init()
{
    if(mountSysPoint.empty())
    {
        ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Warning,"no drive found with QtSystemInformation");
        #ifdef Q_OS_WIN32
        int index=0;
        QFileInfoList drives=QDir::drives();
        while(index<drives.size())
        {
            mountSysPoint<<drives.at(index).absoluteFilePath();
            index++;
        }
        ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Warning,QString("%1 added with QDir::drives()").arg(mountSysPoint.size()));
        #endif
    }
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"mountSysPoint with Qt: "+mountSysPoint.join(";"));
}

PluginInterface_CopyEngine * CopyEngineFactory::getInstance()
{
    CopyEngine *realObject=new CopyEngine(facilityEngine);
    #ifdef ULTRACOPIER_PLUGIN_DEBUG
    connect(realObject,&CopyEngine::debugInformation,this,&CopyEngineFactory::debugInformation);
    #endif
    realObject->connectTheSignalsSlots();
    connect(this,&CopyEngineFactory::haveDrive,realObject,&CopyEngine::setDrive);
    realObject->setDrive(mountSysPoint);
    PluginInterface_CopyEngine * newTransferEngine=realObject;
    connect(this,&CopyEngineFactory::reloadLanguage,realObject,&CopyEngine::newLanguageLoaded);
    realObject->setRightTransfer(ui->doRightTransfer->isChecked());
    realObject->setKeepDate(ui->keepDate->isChecked());
    realObject->setBlockSize(ui->blockSize->value());
    realObject->setAutoStart(ui->autoStart->isChecked());
    realObject->on_comboBoxFolderCollision_currentIndexChanged(ui->comboBoxFolderCollision->currentIndex());
    realObject->on_comboBoxFolderError_currentIndexChanged(ui->comboBoxFolderError->currentIndex());
    realObject->on_comboBoxFileCollision_currentIndexChanged(ui->comboBoxFileCollision->currentIndex());
    realObject->on_comboBoxFileError_currentIndexChanged(ui->comboBoxFileError->currentIndex());
    realObject->setCheckDestinationFolderExists(ui->checkBoxDestinationFolderExists->isChecked());
    realObject->set_doChecksum(ui->doChecksum->isChecked());
    realObject->set_checksumIgnoreIfImpossible(ui->checksumIgnoreIfImpossible->isChecked());
    realObject->set_checksumOnlyOnError(ui->checksumOnlyOnError->isChecked());
    realObject->set_osBuffer(ui->osBuffer->isChecked());
    realObject->set_osBufferLimited(ui->osBufferLimited->isChecked());
    realObject->set_osBufferLimit(ui->osBufferLimit->value());
    realObject->set_setFilters(includeStrings,includeOptions,excludeStrings,excludeOptions);
    realObject->setRenamingRules(firstRenamingRule,otherRenamingRule);
    return newTransferEngine;
}

void CopyEngineFactory::setResources(OptionInterface * options,const QString &writePath,const QString &pluginPath,FacilityInterface * facilityInterface,const bool &portableVersion)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"start, writePath: "+writePath+", pluginPath:"+pluginPath);
    this->facilityEngine=facilityInterface;
    Q_UNUSED(portableVersion);
    #ifndef ULTRACOPIER_PLUGIN_DEBUG
        Q_UNUSED(writePath);
        Q_UNUSED(pluginPath);
    #endif
    #if ! defined (Q_CC_GNU)
        ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Information,"Unable to change date time of files, only gcc is supported");
    #endif
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Information,COMPILERINFO);
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Information,"MAX BUFFER BLOCK: "+QString::number(ULTRACOPIER_PLUGIN_MAXBUFFERBLOCK));
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Information,"MIN TIMER INTERVAL: "+QString::number(ULTRACOPIER_PLUGIN_MINTIMERINTERVAL));
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Information,"MAX TIMER INTERVAL: "+QString::number(ULTRACOPIER_PLUGIN_MAXTIMERINTERVAL));
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Information,"NUM SEM SPEED MANAGEMENT: "+QString::number(ULTRACOPIER_PLUGIN_NUMSEMSPEEDMANAGEMENT));
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Information,"MAX PARALLEL INODE OPT: "+QString::number(ULTRACOPIER_PLUGIN_MAXPARALLELINODEOPT));
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Information,"MAX PARALLEL TRANFER: "+QString::number(ULTRACOPIER_PLUGIN_MAXPARALLELTRANFER));
    #if defined (ULTRACOPIER_PLUGIN_CHECKLISTTYPE)
        ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Information,"CHECK LIST TYPE set");
    #else
        ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Information,"CHECK LIST TYPE not set");
    #endif
    if(options!=NULL)
    {
        //load the options
        QList<QPair<QString, QVariant> > KeysList;
        KeysList.append(qMakePair(QString("doRightTransfer"),QVariant(false)));
        KeysList.append(qMakePair(QString("keepDate"),QVariant(false)));
        KeysList.append(qMakePair(QString("blockSize"),QVariant(ULTRACOPIER_PLUGIN_DEFAULT_BLOCK_SIZE)));//4KB as default
        KeysList.append(qMakePair(QString("autoStart"),QVariant(true)));
        KeysList.append(qMakePair(QString("folderError"),QVariant(0)));
        KeysList.append(qMakePair(QString("folderCollision"),QVariant(0)));
        KeysList.append(qMakePair(QString("fileError"),QVariant(0)));
        KeysList.append(qMakePair(QString("fileCollision"),QVariant(0)));
        KeysList.append(qMakePair(QString("checkDestinationFolder"),QVariant(true)));
        KeysList.append(qMakePair(QString("includeStrings"),QVariant(QStringList())));
        KeysList.append(qMakePair(QString("includeOptions"),QVariant(QStringList())));
        KeysList.append(qMakePair(QString("excludeStrings"),QVariant(QStringList())));
        KeysList.append(qMakePair(QString("excludeOptions"),QVariant(QStringList())));
        KeysList.append(qMakePair(QString("doChecksum"),QVariant(false)));
        KeysList.append(qMakePair(QString("checksumIgnoreIfImpossible"),QVariant(true)));
        KeysList.append(qMakePair(QString("checksumOnlyOnError"),QVariant(true)));
        KeysList.append(qMakePair(QString("osBuffer"),QVariant(false)));
        KeysList.append(qMakePair(QString("firstRenamingRule"),QVariant("")));
        KeysList.append(qMakePair(QString("otherRenamingRule"),QVariant("")));
        KeysList.append(qMakePair(QString("osBufferLimited"),QVariant(false)));
        KeysList.append(qMakePair(QString("osBufferLimit"),QVariant(512)));
        options->addOptionGroup(KeysList);
        #if ! defined (Q_CC_GNU)
        ui->keepDate->setEnabled(false);
        ui->keepDate->setToolTip("Not supported with this compiler");
        #endif
        ui->doRightTransfer->setChecked(options->getOptionValue("doRightTransfer").toBool());
        ui->keepDate->setChecked(options->getOptionValue("keepDate").toBool());
        ui->blockSize->setValue(options->getOptionValue("blockSize").toUInt());
        ui->autoStart->setChecked(options->getOptionValue("autoStart").toBool());
        ui->comboBoxFolderError->setCurrentIndex(options->getOptionValue("folderError").toUInt());
        ui->comboBoxFolderCollision->setCurrentIndex(options->getOptionValue("folderCollision").toUInt());
        ui->comboBoxFileError->setCurrentIndex(options->getOptionValue("fileError").toUInt());
        ui->comboBoxFileCollision->setCurrentIndex(options->getOptionValue("fileCollision").toUInt());
        ui->checkBoxDestinationFolderExists->setChecked(options->getOptionValue("checkDestinationFolder").toBool());

        ui->doChecksum->setChecked(options->getOptionValue("doChecksum").toBool());
        ui->checksumIgnoreIfImpossible->setChecked(options->getOptionValue("checksumIgnoreIfImpossible").toBool());
        ui->checksumOnlyOnError->setChecked(options->getOptionValue("checksumOnlyOnError").toBool());

        ui->osBuffer->setChecked(options->getOptionValue("osBuffer").toBool());
        ui->osBufferLimited->setChecked(options->getOptionValue("osBufferLimited").toBool());
        ui->osBufferLimit->setValue(options->getOptionValue("osBufferLimit").toUInt());
        //ui->autoStart->setChecked(options->getOptionValue("autoStart").toBool());//moved from options(), wrong previous place
        includeStrings=options->getOptionValue("includeStrings").toStringList();
        includeOptions=options->getOptionValue("includeOptions").toStringList();
        excludeStrings=options->getOptionValue("excludeStrings").toStringList();
        excludeOptions=options->getOptionValue("excludeOptions").toStringList();
        filters->setFilters(includeStrings,includeOptions,excludeStrings,excludeOptions);
        firstRenamingRule=options->getOptionValue("firstRenamingRule").toString();
        otherRenamingRule=options->getOptionValue("otherRenamingRule").toString();
        renamingRules->setRenamingRules(firstRenamingRule,otherRenamingRule);

        ui->checksumOnlyOnError->setEnabled(ui->doChecksum->isChecked());
        ui->checksumIgnoreIfImpossible->setEnabled(ui->doChecksum->isChecked());

        updateBufferCheckbox();
        optionsEngine=options;
    }
}

QStringList CopyEngineFactory::supportedProtocolsForTheSource()
{
    return QStringList() << "file";
}

QStringList CopyEngineFactory::supportedProtocolsForTheDestination()
{
    return QStringList() << "file";
}

Ultracopier::CopyType CopyEngineFactory::getCopyType()
{
    return Ultracopier::FileAndFolder;
}

Ultracopier::TransferListOperation CopyEngineFactory::getTransferListOperation()
{
    return Ultracopier::TransferListOperation_ImportExport;
}

bool CopyEngineFactory::canDoOnlyCopy()
{
    return false;
}

void CopyEngineFactory::resetOptions()
{
}

QWidget * CopyEngineFactory::options()
{
    return tempWidget;
}

void CopyEngineFactory::setDoRightTransfer(bool doRightTransfer)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"the value have changed");
    if(optionsEngine!=NULL)
        optionsEngine->setOptionValue("doRightTransfer",doRightTransfer);
}

void CopyEngineFactory::setKeepDate(bool keepDate)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"the value have changed");
    if(optionsEngine!=NULL)
        optionsEngine->setOptionValue("keepDate",keepDate);
}

void CopyEngineFactory::setBlockSize(int blockSize)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"the value have changed");
    if(optionsEngine!=NULL)
        optionsEngine->setOptionValue("blockSize",blockSize);
}

void CopyEngineFactory::setAutoStart(bool autoStart)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"the value have changed");
    if(optionsEngine!=NULL)
        optionsEngine->setOptionValue("autoStart",autoStart);
}

void CopyEngineFactory::setFolderCollision(int index)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"the value have changed");
    if(optionsEngine!=NULL)
        optionsEngine->setOptionValue("folderCollision",index);
}

void CopyEngineFactory::setFolderError(int index)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"the value have changed");
    if(optionsEngine!=NULL)
        optionsEngine->setOptionValue("folderError",index);
}

void CopyEngineFactory::setCheckDestinationFolder()
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"the value have changed");
    if(optionsEngine!=NULL)
        optionsEngine->setOptionValue("checkDestinationFolder",ui->checkBoxDestinationFolderExists->isChecked());
}

void CopyEngineFactory::newLanguageLoaded()
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"start, retranslate the widget options");
    OptionInterface * optionsEngine=this->optionsEngine;
    this->optionsEngine=NULL;
    ui->retranslateUi(tempWidget);
    ui->comboBoxFolderError->setItemText(0,tr("Ask"));
    ui->comboBoxFolderError->setItemText(1,tr("Skip"));

    ui->comboBoxFolderCollision->setItemText(0,tr("Ask"));
    ui->comboBoxFolderCollision->setItemText(1,tr("Merge"));
    ui->comboBoxFolderCollision->setItemText(2,tr("Skip"));
    ui->comboBoxFolderCollision->setItemText(3,tr("Rename"));

    ui->comboBoxFileError->setItemText(0,tr("Ask"));
    ui->comboBoxFileError->setItemText(1,tr("Skip"));
    ui->comboBoxFileError->setItemText(2,tr("Put at the end"));

    ui->comboBoxFileCollision->setItemText(0,tr("Ask"));
    ui->comboBoxFileCollision->setItemText(1,tr("Skip"));
    ui->comboBoxFileCollision->setItemText(2,tr("Overwrite"));
    ui->comboBoxFileCollision->setItemText(3,tr("Overwrite if different"));
    ui->comboBoxFileCollision->setItemText(4,tr("Overwrite if newer"));
    ui->comboBoxFileCollision->setItemText(5,tr("Overwrite if older"));
    ui->comboBoxFileCollision->setItemText(6,tr("Rename"));
    if(optionsEngine!=NULL)
    {
        filters->newLanguageLoaded();
        renamingRules->newLanguageLoaded();
    }
    emit reloadLanguage();
    this->optionsEngine=optionsEngine;
}

void CopyEngineFactory::doChecksum_toggled(bool doChecksum)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"the value have changed");
    if(optionsEngine!=NULL)
        optionsEngine->setOptionValue("doChecksum",doChecksum);
}

void CopyEngineFactory::checksumOnlyOnError_toggled(bool checksumOnlyOnError)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"the value have changed");
    if(optionsEngine!=NULL)
        optionsEngine->setOptionValue("checksumOnlyOnError",checksumOnlyOnError);
}

void CopyEngineFactory::osBuffer_toggled(bool osBuffer)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"the value have changed");
    if(optionsEngine!=NULL)
        optionsEngine->setOptionValue("osBuffer",osBuffer);
    ui->osBufferLimit->setEnabled(ui->osBuffer->isChecked() && ui->osBufferLimited->isChecked());
}

void CopyEngineFactory::osBufferLimited_toggled(bool osBufferLimited)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"the value have changed");
    if(optionsEngine!=NULL)
        optionsEngine->setOptionValue("osBufferLimited",osBufferLimited);
    ui->osBufferLimit->setEnabled(ui->osBuffer->isChecked() && ui->osBufferLimited->isChecked());
}

void CopyEngineFactory::osBufferLimit_editingFinished()
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"the spinbox have changed");
    if(optionsEngine!=NULL)
        optionsEngine->setOptionValue("osBufferLimit",ui->osBufferLimit->value());
}

void CopyEngineFactory::showFilterDialog()
{
    if(optionsEngine==NULL)
    {
        QMessageBox::critical(NULL,tr("Options error"),tr("Options engine is not loaded, can't access to the filters"));
        ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Critical,"options not loaded");
        return;
    }
    filters->exec();
}

void CopyEngineFactory::sendNewFilters(const QStringList &includeStrings,const QStringList &includeOptions,const QStringList &excludeStrings,const QStringList &excludeOptions)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"new filter");
    this->includeStrings=includeStrings;
    this->includeOptions=includeOptions;
    this->excludeStrings=excludeStrings;
    this->excludeOptions=excludeOptions;
    if(optionsEngine!=NULL)
    {
        optionsEngine->setOptionValue("includeStrings",includeStrings);
        optionsEngine->setOptionValue("includeOptions",includeOptions);
        optionsEngine->setOptionValue("excludeStrings",excludeStrings);
        optionsEngine->setOptionValue("excludeOptions",excludeOptions);
    }
}

void CopyEngineFactory::sendNewRenamingRules(const QString &firstRenamingRule,const QString &otherRenamingRule)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"new filter");
    this->firstRenamingRule=firstRenamingRule;
    this->otherRenamingRule=otherRenamingRule;
    if(optionsEngine!=NULL)
    {
        optionsEngine->setOptionValue("firstRenamingRule",firstRenamingRule);
        optionsEngine->setOptionValue("otherRenamingRule",otherRenamingRule);
    }
}

void CopyEngineFactory::showRenamingRules()
{
    if(optionsEngine==NULL)
    {
        QMessageBox::critical(NULL,tr("Options error"),tr("Options engine is not loaded, can't access to the filters"));
        ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Critical,"options not loaded");
        return;
    }
    renamingRules->exec();
}

void CopyEngineFactory::updateBufferCheckbox()
{
    ui->osBufferLimited->setEnabled(ui->osBuffer->isChecked());
    ui->osBufferLimit->setEnabled(ui->osBuffer->isChecked() && ui->osBufferLimited->isChecked());
}

void CopyEngineFactory::checksumIgnoreIfImpossible_toggled(bool checksumIgnoreIfImpossible)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"the value have changed");
    if(optionsEngine!=NULL)
        optionsEngine->setOptionValue("checksumIgnoreIfImpossible",checksumIgnoreIfImpossible);
}

void CopyEngineFactory::logicalDriveChanged(const QString &,bool)
{
    QStringList temp=storageInfo.allLogicalDrives();
    for (int i = 0; i < temp.size(); ++i) {
        mountSysPoint<<QDir::toNativeSeparators(temp.at(i));
    }
    if(mountSysPoint.empty())
    {
        ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Warning,"no drive found with QtSystemInformation");
        #ifdef Q_OS_WIN32
        int index=0;
        QFileInfoList drives=QDir::drives();
        while(index<drives.size())
        {
            mountSysPoint<<drives.at(index).absoluteFilePath();
            index++;
        }
        ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Warning,QString("%1 added with QDir::drives()").arg(mountSysPoint.size()));
        #endif
    }
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,"mountSysPoint with Qt: "+mountSysPoint.join(";"));
    emit haveDrive(mountSysPoint);
}

void CopyEngineFactory::setFileCollision(int index)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,QString("action index: %1").arg(index));
    if(optionsEngine==NULL)
        return;
    switch(index)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            optionsEngine->setOptionValue("fileCollision",index);
        break;
        default:
            ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Warning,"Error, unknow index, ignored");
        break;
    }
}

void CopyEngineFactory::setFileError(int index)
{
    ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Notice,QString("action index: %1").arg(index));
    if(optionsEngine==NULL)
        return;
    switch(index)
    {
        case 0:
        case 1:
        case 2:
            optionsEngine->setOptionValue("fileError",index);
        break;
        default:
            ULTRACOPIER_DEBUGCONSOLE(Ultracopier::DebugLevel_Warning,"Error, unknow index, ignored");
        break;
    }
}