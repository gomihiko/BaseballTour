#include "admin.h"
#include "ui_admin.h"
#include "dbmanager.h"
#include "addsouvenir.h"
//#include "fileselector.h"

Admin::Admin(dbManager* db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Admin)
{
    m_db = db;

    ui->setupUi(this);
    m_db = db;

    nameList = database->getTeamNames();
    tempList = nameList;

    for (const auto &teamName : nameList)
    {
        ui->comboBox->addItem(teamName);
    }

    // populate the fields on init
    populateInformation(ui->comboBox->currentText());

    updateSouvenirs();
}

Admin::~Admin()
{
    delete ui;
}

void Admin::updateSouvenirs()
{
    QSqlQueryModel* model=new QSqlQueryModel();

    QSqlQuery qry;

    qry.prepare("SELECT * FROM Souvenirs");

    if(qry.exec())
    {
        qDebug() << "Souvenirs updated";
    }

    model->setQuery(qry);

    QSortFilterProxyModel *m=new QSortFilterProxyModel(this);

    m->setDynamicSortFilter(true);
    m->setSourceModel(model);
    ui->souvenir_tableView->setModel(m);
    ui->souvenir_tableView->setColumnWidth(0,180);
}

void Admin::on_pushButton_updateSouvenir_clicked()
{
    bool success = true;

    if(ui->lineEdit_souvenirName->text() == "")
    {
        ui->lineEdit_souvenirName->setPlaceholderText("souvenir name empty!");
        success = false;
    }

    if(!m_db->souvenirExists(ui->lineEdit_souvenirName->text(), ui->label_stadiumName->text()))
    {
        if(success)
        {
            double costToString = ui->doubleSpinBox_cost->value();
            QString cost = QString::number(costToString);
            // cost = cost;
            m_db->updateSouvenir(tempSouvenir, ui->label_stadiumName->text(), cost, ui->lineEdit_souvenirName->text());
            qDebug() << "Souvenir Name: " << ui->lineEdit_souvenirName->text();
            qDebug() << "Temp: " << tempSouvenir;
        }
        else
        {
            qDebug() << "Remove error";
        }
    }
    else if(tempSouvenir == ui->lineEdit_souvenirName->text())
    {
        if(success)
        {
            double costToString = ui->doubleSpinBox_cost->value();
            QString cost = QString::number(costToString);
            m_db->updateSouvenir(tempSouvenir, ui->label_stadiumName->text(), cost, ui->lineEdit_souvenirName->text());
            qDebug() << ui->lineEdit_souvenirName->text();
        }
        else
        {
            qDebug() << "Remove error";
        }
        updateSouvenirs();
    }
    updateSouvenirs();
}

void Admin::on_pushButton_removeSouvenir_clicked()
{
    bool success = true;

    if(ui->lineEdit_souvenirName->text() == "")
    {
        ui->lineEdit_souvenirName->setPlaceholderText("");
        success = false;
    }

    if(!m_db->souvenirExists(ui->lineEdit_souvenirName->text(), ui->label_stadiumName->text()))
    {
       ui->lineEdit_souvenirName->setText("");
       ui->lineEdit_souvenirName->setPlaceholderText("souvenir doesn't exist!");
       success = false;
    }

    if(m_db->souvenirExists(ui->lineEdit_souvenirName->text(), ui->label_stadiumName->text()) && success)
    {
        qDebug() << "Checking if souvenir exists";
        m_db->removeSouvenir(ui->lineEdit_souvenirName->text(), ui->label_stadiumName->text());
        ui->lineEdit_souvenirName->setText("");
        ui->lineEdit_souvenirName->setPlaceholderText("souvenir name");
    }
    else
    {
        qDebug() << "Error removing Souvenir";
    }
    updateSouvenirs();
}

void Admin::on_souvenir_tableView_clicked(const QModelIndex &index)
{
    if(index.isValid())
    {
        int row = index.row();
        qDebug() << row;
        QString teams = index.sibling(row, 0).data().toString();
        qDebug() << teams;
        QString souvenirName = index.sibling(row, 1).data().toString();
        qDebug() << souvenirName;
        tempSouvenir = index.sibling(row, 1).data().toString();
        double thirdText = index.sibling(row, 2).data().toString().replace("$", "").toDouble();

        ui->lineEdit_souvenirName->setText(souvenirName);
        ui->label_stadiumName->setText(teams);
        ui->doubleSpinBox_cost->setValue(thirdText);
    }
}

void Admin::on_pushButton_addNewSouvenir_clicked()
{
    addSouvenir adding;
    adding.setModal(true);
    adding.exec();
    updateSouvenirs();
    qDebug() << "Add souvenir button pressed";
}

void Admin::updateSearchResults(const QString &souvenirName, const QString &team)
{
    QSqlQueryModel* model=new QSqlQueryModel();

    QSqlQuery qry;

    qDebug() << "The souvenir name is: " << souvenirName;

    if(souvenirName == "" && team != "")
    {
        qry.prepare("SELECT teams, souvenirs, cost FROM Souvenirs WHERE (teams) = (:teams)");
        qry.bindValue(":teams", team);
    }
    else if(souvenirName != "" && team == "")
    {
        qry.prepare("SELECT teams, souvenirs, cost FROM Souvenirs WHERE (Souvenirs) = (:souvenirName)");
        qry.bindValue(":souvenirName", souvenirName);
    }
    else
    {
        qry.prepare("SELECT teams, souvenirs, cost FROM Souvenirs WHERE (Souvenirs, teams) = (:souvenirName, :teams)");
        qry.bindValue(":souvenirName", souvenirName);
        qry.bindValue(":teams", team);
    }

    if(qry.exec())
    {
        qDebug() << "Search updated";
    }

    model->setQuery(qry);

    QSortFilterProxyModel *m=new QSortFilterProxyModel(this);

    m->setDynamicSortFilter(true);
    m->setSourceModel(model);
    ui->souvenir_tableView->setModel(m);
    ui->souvenir_tableView->setColumnWidth(0,180);
}

void Admin::on_pushButton_search_clicked()
{
    QString teamName = ui->lineEdit_teamName->text();
    QString souvenirName = ui->lineEdit_souvenirName->text();

    qDebug() << "TEAM: " << teamName;
    qDebug() << "ITEM: " << souvenirName;

    if(teamName != "" && souvenirName == "")
    {
        updateSearchResults(nullptr, teamName);
    }
    else if(teamName == "" && souvenirName != "")
    {
        updateSearchResults(souvenirName, nullptr);
    }
    else if(teamName !=  nullptr && souvenirName != nullptr)
    {
        updateSearchResults(souvenirName, teamName);
    }
}

void Admin::on_pushButton_clicked()
{
    QString teamName = ui->comboBox->currentText();
    QString stadiumName = ui->stadiumName_field->text();
    int capacity = ui->seatingCapacity->text().toInt();
    QString location = ui->stadiumLocation_field->text();
    QString playingSurface = ui->playingSurface_field->text();
    QString league = ui->listWidget->currentItem()->text();
    int date = ui->dateOpened_spinbox->value();
    QString distCenterField = ui->distance_field->text();
    QString typology = ui->typology_field->text();
    QString roofType = ui->roof_field->text();

    m_db->updateTeam(teamName,stadiumName,capacity,location,playingSurface,league,date,distCenterField,typology,roofType);

    QMessageBox::information(this, "Success", "Modified team in database.");


}

void Admin::populateInformation(const QString &teamName)
{
    teamData data = m_db->getTeamData(teamName);

    ui->stadiumName_field->setText(data.stadium_name);
    ui->seatingCapacity->setValue(data.stadium_seating_capacity);
    ui->stadiumLocation_field->setText(data.stadium_location);
    ui->playingSurface_field->setText(data.stadium_playing_surface);

    if(data.team_league == "American") {
        ui->listWidget->setCurrentRow(0);
    }else {
        ui->listWidget->setCurrentRow(1);
    }

    ui->dateOpened_spinbox->setValue(data.stadium_date_opened);
    ui->distance_field->setText(data.stadium_dist_ctrfield);
    ui->typology_field->setText(data.stadium_typology);
    ui->roof_field->setText(data.stadium_roof_type);
}

void Admin::on_comboBox_currentIndexChanged(const QString &arg1)
{
    qDebug() << "populating information from:" << arg1;
    populateInformation(arg1);
}
