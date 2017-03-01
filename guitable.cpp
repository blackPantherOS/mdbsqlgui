#include "guitable.h"
#include "ui_guitable.h"
#include <QListWidgetItem>
#include <QInputDialog>
#include <QComboBox>
#include <QLabel>
#include <QMessageBox>
#include <settings.h>

void GuiTable::newColumnSlot()
{
    ui->ColumnName->clear();
    ui->ColumnType->setCurrentText("");
    ui->ColumnDefaultValue->clear();
    ui->ColumnPrimaryKey->setChecked(false);
    ui->ColumnNotNull->setChecked(false);
    ui->ColumnUnique->setChecked(false);
    ui->ColumnAutoIncrement->setChecked(false);
    ui->ColumnComment->clear();
    ui->ColumnCollation->setCurrentText("");

    //Focus
    ui->ColumnName->setFocus();
}
void GuiTable::newFokSlot()
{
    ui->FokColumn->clear();
    ui->FokName->clear();
    ui->FokTable->clear();
}
void GuiTable::deleteColumnSlot()
{
    QList<QListWidgetItem *> items = ui->Columns->selectedItems();
    QListWidgetItem * item;
    QJsonObject cols;
    if (Data.contains(".columns"))
        cols = Data[".columns"].toObject();
    QJsonObject col;
    QJsonArray toDelete;
    if (Data.contains(".columnsToDelete"))
        toDelete = Data[".columnsToDelete"].toArray();
    QString cname;
    for (int i=0;i<items.count();i++)
    {
        item = items.at(i);
        cname = item->text();
        if (cols.contains(cname))
            cols.remove(cname);
        toDelete.append(cname);
    }
    Data[".columns"] = QJsonObject(cols);
    if (Alter)
        Data[".columnsToDelete"] = toDelete;
    SelectColumn.clear();
    showData();
    if (cols.count() == 0)
        newColumnSlot();
}
void GuiTable::deleteFokSlot()
{
    QList<QListWidgetItem *> items = ui->ForeignKeys->selectedItems();
    QListWidgetItem * item;
    QJsonObject constrs;
    if (Data.contains(".contraints"))
        constrs = Data[".contraints"].toObject();
    QJsonObject col;
    QJsonArray toDelete;
    if (Data.contains(".contraintsToDelete"))
        toDelete = Data[".contraintsToDelete"].toArray();
    QString cname;
    for (int i=0;i<items.count();i++)
    {
        item = items.at(i);
        cname = item->text();
        if (constrs.contains(cname))
            constrs.remove(cname);
        toDelete.append(cname);
    }
    Data[".contraints"] = QJsonObject(constrs);
    if (Alter)
        Data[".contraintsToDelete"] = toDelete;
    SelectFok.clear();
    showData();
    if (constrs.count() == 0)
        newFokSlot();
}
void GuiTable::setColumnSlot()
{
    if (!validateColumnData())
        return;
    setData(true);
    QJsonObject cols;
    bool modify = false;
    QJsonObject toAdd;
    QJsonObject toModify;
    if (Data.contains(".columns"))
        cols = Data[".columns"].toObject();
    if (Data.contains(".columnsToAdd"))
        toAdd = Data[".columnsToAdd"].toObject();
    if (Data.contains(".columnsToModify"))
        toModify = Data[".columnsToModify"].toObject();
    QJsonObject col;
    QString cname = ui->ColumnName->text();
    if (cols.contains(cname))
    {
        modify = true;
        col = cols[cname].toObject();
    }
    QString ctype = ui->ColumnType->currentText();
    col["COLUMN_TYPE"] = ctype;
    col[".NotNull"] = ui->ColumnNotNull->isChecked();
    col[".Unique"] = ui->ColumnUnique->isChecked();
    col[".PrimaryKey"] = ui->ColumnPrimaryKey->isChecked();
    col[".AutoIncrement"] = ui->ColumnAutoIncrement->isChecked();
    col["COLUMN_DEFAULT"] = ui->ColumnDefaultValue->text();
    col["COLUMN_COMMENT"] = ui->ColumnComment->text();
    col["COLLATION_NAME"] = ui->ColumnCollation->currentText();
    cols[cname] = col;
    if (Alter)
    {
        if (modify)
        {
            toModify[cname] = col;
            Data[".columnsToModify"] = toModify;
        }
        else
        {
            toAdd[cname] = col;
            Data[".columnsToAdd"] = toAdd;
        }
    }
    SelectColumn = cname;
    Data[".columns"] = QJsonObject(cols);    
    if (!Settings::ColumnTypes.contains(ctype))
    {
        Settings::ColumnTypes.append(ctype);
        ui->ColumnType->addItem(ctype);
    }
    showData();
}
void GuiTable::setFokSlot()
{
    if (!validateColumnData())
        return;
    setData(true);
    QJsonObject constraints;
    bool modify = false;
    QJsonObject toAdd;
    QJsonObject toModify;
    if (Data.contains(".constraints"))
        constraints = Data[".constraints"].toObject();
    if (Data.contains(".constraintsToAdd"))
        toAdd = Data[".constraintsToAdd"].toObject();
    if (Data.contains(".constraintsToModify"))
        toModify = Data[".constraintsToModify"].toObject();
    QJsonObject constraint;
    QString cname = ui->FokName->text();
    if (constraints.contains(cname))
    {
        modify = true;
        constraint = constraints[cname].toObject();
    }
    constraint["Name"] = cname;
    constraint["Table"] = ui->FokTable->currentText();
    constraint["Column"] = ui->FokColumn->currentText();
    constraints[cname] = constraint;
    if (Alter)
    {
        if (modify)
        {
            toModify[cname] = constraint;
            Data[".constraintsToModify"] = toModify;
        }
        else
        {
            toAdd[cname] = constraint;
            Data[".constraintsToAdd"] = toAdd;
        }
    }
    SelectColumn = cname;
    Data[".constraints"] = QJsonObject(constraints);
    showData();
}
void GuiTable::loadFromTemplateSlot()
{
    QStringList tmpls = Settings::TableTemplates.keys();
    if (tmpls.count() == 0)
    {
        errMsg("No templates available!");
        return;
    }
    bool ok = false;
    QString tmplname = QInputDialog::getItem(this, tr("Template selection"), tr("Select template"), tmpls, 0, false, &ok);
    if (!ok)
        return;
    Data = Settings::TableTemplates[tmplname].toObject();
    showData();
}
void GuiTable::removeTemplateSlot()
{
    QStringList tmpls = Settings::TableTemplates.keys();
    if (tmpls.count() == 0)
    {
        errMsg("No templates available!");
        return;
    }
    bool ok = false;
    QString tmplname = QInputDialog::getItem(this, tr("Template selection"), tr("Select template to remove"), tmpls, 0, false, &ok);
    if (!ok)
        return;
    Settings::TableTemplates.remove(tmplname);
    Settings::sync();
}
void GuiTable::saveAsTemplateSlot()
{
    bool ok = false;
    if (!validateData())
        return;
    setData();
    QString tmplName;
    while (!ok)
    {
        QStringList tmpls = Settings::TableTemplates.keys();
        tmpls.insert(0, tr("Template%1").arg(Settings::TableTemplates.count()+1));
        tmplName = QInputDialog::getItem(this, tr("Table selection"), tr("Name of table template"), tmpls, 0, true, &ok);
        if (!ok)
            return;
        ok = false;
        //tmplName = QInputDialog::getText(this, tr("Table template"), tr("Name of table template"),QLineEdit::Normal, tr("Template%1").arg(Settings::TableTemplates.count()+1));
        if (Settings::TableTemplates.contains(tmplName))
        {
            int rc = QMessageBox::question(this, tr("Template confirmation"), tr("Table template with name '%1' already exists, Do You want replace it ?").arg(tmplName), QMessageBox::No, QMessageBox::Yes);
            if (rc == (int)QMessageBox::Yes)
                ok = true;
        }
        else
            ok = true;
    }
    Settings::TableTemplates[tmplName] = Data;
    Settings::sync();
}

void GuiTable::applySlot()
{
    setData();
    if (!validateData())
        return;

    QStringList sqls = sqlCommand();
    QString sql;

    QSqlDatabase db = QSqlDatabase::database(Ent->connection());
    QSqlQuery qry(db);
    QSqlError err;
    Con->runSql(QString("USE %1;").arg(Ent->database()), &qry, &err);
    Con->runSql("START TRANSACTION;", &qry, &err);
    bool hasErr = false;
    QString errors;
    for (int i=0;i<sqls.count();i++)
    {
        sql = sqls.at(i);
        if (!Con->runSql(sql, &qry, &err))
        {
            if (!errors.isEmpty())
                errors += "\n";
            errors += err.databaseText();
            hasErr = true;
        }
    }

    if (hasErr)
    {
        Con->runSql("ROLLBACK;", &qry, &err);
        QMessageBox::critical(this, tr("Error"), errors);
        return;
    }

    Con->runSql("COMMIT;", &qry, &err);
    emit dbRefresh();
    QString msg = "";
    if (Create)
        msg = tr("Table %1 was successfully created").arg(Data["Name"].toString());
    if (Alter)
        msg = tr("Table %1 was successfully changed").arg(Data["Name"].toString());
    QMessageBox::information(this, tr("success"), msg);
}

bool GuiTable::validateColumnData()
{
    if (ui->ColumnName->text().isEmpty())
    {
        errMsg(tr("Column name is empty!"));
        ui->ColumnName->setFocus();
        return false;
    }
    if (ui->ColumnType->currentText().isEmpty())
    {
        errMsg(tr("Column type is empty!"));
        ui->ColumnType->setFocus();
        return false;
    }
    errMsg();
    return true;
}
bool GuiTable::validateFokData()
{
    if (ui->FokTable->currentText().isEmpty())
    {
        errMsg(tr("Foreign table is empty"));
        ui->FokTable->setFocus();
        return false;
    }
    if (ui->FokColumn->currentText().isEmpty())
    {
        errMsg(tr("Foreign column is empty"));
        ui->FokColumn->setFocus();
        return false;
    }
    errMsg();
    return false;
}
bool GuiTable::validateData()
{
    if (ui->TableName->text().isEmpty())
    {
        errMsg(tr("Table name is empty!"));
        ui->TableName->setFocus();
        return false;
    }
    if (ui->Columns->count() == 0)
    {
        errMsg(tr("Least one column is required"));
        newColumnSlot();
        return false;
    }
    errMsg();
    return true;
}
void GuiTable::showSqlCmdSlot()
{
    setData();
    if (!validateData())
        return;
    QStringList sqls = sqlCommand();
    QString sql;
    for (int i=0;i<sqls.count();i++)
    {
        if (!sql.isEmpty())
            sql += "\n\n";
        sql += sqls.at(i);
    }
    QInputDialog::getMultiLineText(this, tr("Sql command"), "Sql", sql);
}
void GuiTable::columnActivatedSlot(QListWidgetItem *row)
{
    QString cname = row->text();
    SelectColumn = cname;
    showColumnData(cname);
}
void GuiTable::fokActivatedSlot(QListWidgetItem *row)
{
    QString fokname = row->text();

}
void GuiTable::errMsg(QString msg)
{
    if (msg.isEmpty())
    {
        ui->Msg->clear();        
        return;
    }    
    ui->Msg->setText(msg);
}
QStringList GuiTable::sqlCommand()
{
    QStringList sqls;
    QString key, tmp;
    QJsonObject iobj;
    QString idef;
    if (Create)
    {
        QString sqlCreate = QString("CREATE TABLE `%1` ( \n").arg(Data["TABLE_NAME"].toString());
        QJsonObject cols = Data[".columns"].toObject();
        QStringList keys = cols.keys();
        for (int i=0;i<keys.count();i++)
        {
            key = keys.at(i);
            iobj = cols[key].toObject();

            // Name
            idef = "`"+key+"`";

            // Type
            idef += " " + iobj["COLUMN_TYPE"].toString();

            // NotNull
            if (iobj[".NotNull"].toBool())
                idef += " NOT NULL";
            else
                idef += " NULL";

            // Default Value
            tmp = iobj["COLUMN_DEFAULT"].toString();
            if (!tmp.isEmpty())
                idef += " DEFAULT " + tmp;

            // AutoIncrement
            if (iobj[".AutoIncrement"].toBool())
                idef += " AUTO_INCREMENT";

            // Unique
            if (iobj[".Unique"].toBool())
                idef += " UNIQUE KEY";

            // PrimaryKey
            if (iobj[".PrimaryKey"].toBool())
                idef += " PRIMARY KEY";

            // Collation
            tmp = iobj["COLLATION_NAME"].toString();
            if (!tmp.isEmpty())
                idef += QString(" COLLATE '%1'").arg(tmp);

            // Comment
            tmp = iobj["COLUMN_COMMENT"].toString();
            if (!tmp.isEmpty())
                idef += QString(" COMMENT '%1'").arg(tmp);

            sqlCreate += idef;
            if (i < keys.count() -1)
                sqlCreate += ",\n";
        }
        sqlCreate += QString("\n )\nENGINE=%1 DEFAULT COLLATE='%2' COMMENT='%3';")
                .arg(Data["ENGINE"].toString())
                .arg(Data["TABLE_COLLATION"].toString())
                .arg(Data["TABLE_COMMENT"].toString());
        sqls.append(sqlCreate);
    }
    else
    {
        QString sqlTab;
        QString sqlDrop;
        QString sqlModify;
        QString sqlAdd;
        QString cname;
        QStringList alterTableParams = tableAlter();

        if (alterTableParams.count() > 0)
        {
            sqlTab = QString("ALTER TABLE `%1`").arg(Data["TABLE_NAME"].toString());
            QString param;
            for (int i=0;i<alterTableParams.count();i++)
            {
                param = alterTableParams.at(i);
                if (param == "ENGINE")
                    sqlTab += QString(" ENGINE=%1").arg(Data["ENGINE"].toString());
                else if (param == "TABLE_COLLATION")
                    sqlTab += QString(" COLLATE='%1'").arg(Data["TABLE_COLLATION"].toString());
                else if (param == "TABLE_COMMENT")
                    sqlTab += QString(" COMMENT='%1'").arg(Data["TABLE_COMMENT"].toString());
            }
            sqlTab += ";";
            sqls.append(sqlTab);
        }


        QJsonArray toDelete = Data[".columnsToDelete"].toArray();
        if (toDelete.count() > 0)
        {
            sqlDrop = QString("ALTER TABLE `%1`").arg(Data["TABLE_NAME"].toString());
            for (int i=0;i<toDelete.count();i++)
            {
                tmp = toDelete.at(i).toString();
                if (!sqlDrop.isEmpty())
                    sqlDrop += " ";
                sqlDrop += QString("\nDROP COLUMN `%1`").arg(tmp);
            }
            sqlDrop += ";";
            sqls.append(sqlDrop);
        }

        QStringList keys;
        QJsonObject toModify = Data[".columnsToModify"].toObject();
        keys = toModify.keys();
        if (keys.count() > 0)
        {
            sqlModify =  QString("ALTER TABLE `%1`").arg(Data["TABLE_NAME"].toString());
            for (int i=0;i<keys.count();i++)
            {
                cname = keys.at(i);
                iobj = toModify[cname].toObject();
                // Name
                idef = "`"+cname+"`";

                // Type
                idef += " " + iobj["COLUMN_TYPE"].toString();

                // NotNull
                if (iobj[".NotNull"].toBool())
                    idef += " NOT NULL";
                else
                    idef += " NULL";

                // Default Value
                tmp = iobj["COLUMN_DEFAULT"].toString();
                if (!tmp.isEmpty())
                    idef += " DEFAULT " + tmp;

                // AutoIncrement
                if (iobj[".AutoIncrement"].toBool())
                    idef += " AUTO_INCREMENT";

                // Unique
                if (iobj[".Unique"].toBool())
                    idef += " UNIQUE KEY";

                // PrimaryKey
                if (iobj[".PrimaryKey"].toBool())
                    idef += " PRIMARY KEY";

                // Collation
                tmp = iobj["COLLATION_NAME"].toString();
                if (!tmp.isEmpty())
                    idef += QString(" COLLATE '%1'").arg(tmp);

                // Comment
                tmp = iobj["COLUMN_COMMENT"].toString();
                if (!tmp.isEmpty())
                    idef += QString(" COMMENT '%1'").arg(tmp);

                if (!sqlModify.isEmpty())
                    sqlModify += " ";
                sqlModify += QString("\nMODIFY COLUMN %1").arg(idef);
            }
            sqlModify += ";";
            sqls.append(sqlModify);
        }

        QJsonObject toAdd = Data[".columnsToAdd"].toObject();
        keys = toAdd.keys();
        if (keys.count() > 0)
        {
            sqlAdd =  QString("ALTER TABLE `%1`").arg(Data["TABLE_NAME"].toString());
            for (int i=0;i<keys.count();i++)
            {
                cname = keys.at(i);
                iobj = toAdd[cname].toObject();
                // Name
                idef = "`"+cname+"`";

                // Type
                idef += " " + iobj["COLUMN_TYPE"].toString();

                // NotNull
                if (iobj[".NotNull"].toBool())
                    idef += " NOT NULL";
                else
                    idef += " NULL";

                // Default Value
                tmp = iobj["COLUMN_DEFAULT"].toString();
                if (!tmp.isEmpty())
                    idef += " DEFAULT " + tmp;

                // AutoIncrement
                if (iobj[".AutoIncrement"].toBool())
                    idef += " AUTO_INCREMENT";

                // Unique
                if (iobj[".Unique"].toBool())
                    idef += " UNIQUE KEY";

                // PrimaryKey
                if (iobj[".PrimaryKey"].toBool())
                    idef += " PRIMARY KEY";

                // Collation
                tmp = iobj["COLLATION_NAME"].toString();
                if (!tmp.isEmpty())
                    idef += QString(" COLLATE '%1'").arg(tmp);

                // Comment
                tmp = iobj["COLUMN_COMMENT"].toString();
                if (!tmp.isEmpty())
                    idef += QString(" COMMENT '%1'").arg(tmp);
                if (!sqlAdd.isEmpty())
                    sqlAdd += " ";
                sqlAdd += QString("\nADD COLUMN %1").arg(idef);
            }
            sqlAdd += ";";
            sqls.append(sqlAdd);
        }
    }
    return sqls;
}
QStringList GuiTable::tableAlter()
{
    QStringList changed;
    QString varD, varOD;

    varD = Data["TABLE_NAME"].toString();
    varOD = OriginData["TABLE_NAME"].toString();
    if (varD != varOD)
        changed.append("TABLE_NAME");

    varD = Data["ENGINE"].toString();
    varOD = OriginData["ENGINE"].toString();
    if (varD != varOD)
        changed.append("ENGINE");

    varD = Data["TABLE_COLLATION"].toString();
    varOD = OriginData["TABLE_COLLATION"].toString();
    if (varD != varOD)
        changed.append("TABLE_COLLATION");

    varD = Data["TABLE_COMMENT"].toString();
    varOD = OriginData["TABLE_COMMENT"].toString();
    if (varD != varOD)
        changed.append("TABLE_COMMENT");
    return changed;
}
void GuiTable::setData(bool disable_validate)
{
    if (!disable_validate)
        if (!validateData())
            return;
    Data["TABLE_NAME"] = ui->TableName->text();
    Data[".Temporary"] = ui->TableTemporary->isChecked();
    Data["ENGINE"] = ui->TableEngine->currentText();
    Data["TABLE_COLLATION"] = ui->TableCollation->currentText();
    Data["TABLE_COMMENT"] = ui->TableComment->toPlainText();
}
void GuiTable::showData()
{
    if (Create)
    {
        ui->Apply->setText(tr("Create"));
        ui->TableName->setText(Data["TABLE_NAME"].toString());
        ui->TableWarning->setHidden(true);
    }
    if (Alter)
    {
        ui->Apply->setText(tr("Alter"));
        ui->TableName->setText(TableName);
        ui->TableName->setReadOnly(true);
        ui->TableTemporary->setHidden(true);        
        ui->TableWarning->setText(tr("Table contains %1 row(s)").arg(Data["TABLE_ROWS"].toString()));
    }

    ui->TableTemporary->setChecked(Data[".Temporary"].toBool());
    ui->TableEngine->setCurrentText(Data["ENGINE"].toString());
    ui->TableCollation->setCurrentText(Data["TABLE_COLLATION"].toString());
    ui->TableComment->setPlainText(Data["TABLE_COMMENT"].toString());
    QJsonObject cols, col, idxs, cntrs, cnt;
    if (Data.contains(".columns"))
        cols = Data[".columns"].toObject();
    if (Data.contains(".indexes"))
        idxs = Data[".indexes"].toObject();
    if (Data.contains(".constraints"))
        cntrs = Data[".constraints"].toObject();

    //columns
    QStringList keys = cols.keys();
    QString cname;
    ui->Columns->clear();
    int sci = -1;
    bool pk=false;
    QList<QListWidgetItem *> lwis;
    QListWidgetItem * lwi = 0L;
    for (int i=0;i<keys.count();i++)
    {
        pk=false;
        cname = keys.at(i);
        col = cols[cname].toObject();
        //if (SelectColumn == cname)
        //    sci = i;
        //ui->Columns->addItem(cname);
        //QListWidgetItem * lwi = ui->Columns->item(ui->Columns->count()-1);
        lwi = new QListWidgetItem(cname);
        lwi->setIcon(QIcon(":/icons/column.png"));
        pk = col[".PrimaryKey"].toBool();
        if (pk)
            lwi->setIcon(QIcon(":/icons/primary-key.png"));
        QFont f = lwi->font();
        if (idxs.contains(cname))
            f.setUnderline(true);
        lwi->setFont(f);
        if (pk)
            lwis.insert(0, lwi);
        else
            lwis.append(lwi);
    }

    for (int i=0;i<lwis.count();i++)
    {
        lwi = lwis.at(i);
        cname = lwi->text();
        if (SelectColumn == cname)
            sci = i;
        ui->Columns->addItem(lwi);
    }

    if (sci >=0 && sci < ui->Columns->count())
        ui->Columns->setCurrentRow(sci);

    //foreingKeys
    keys = cntrs.keys();
    QString fokName;
    ui->ForeignKeys->clear();
    sci = -1;
    for (int i=0;i<keys.count();i++)
    {
        fokName = keys.at(i);
        cnt = cntrs[fokName].toObject();
        if (SelectFok == fokName)
            sci = i;
        ui->ForeignKeys->addItem(fokName);
        QListWidgetItem * lwi = ui->ForeignKeys->item(ui->ForeignKeys->count()-1);
        lwi->setIcon(QIcon(":/icons/foreign-key.png"));
    }
}
void GuiTable::makeTableData()
{
    QSqlDatabase db = QSqlDatabase::database(Ent->connection());
    QSqlQuery qry(db);
    QSqlError err;
    Con->runSql(QString("USE information_schema;"), &qry, &err);
    QString tabSql = QString("SELECT * FROM `information_schema`.`TABLES` WHERE TABLE_SCHEMA='%1' AND TABLE_NAME='%2';").arg(Ent->database()).arg(TableName);
    QString colSql = QString("SELECT * FROM `information_schema`.`COLUMNS` WHERE TABLE_SCHEMA='%1' AND TABLE_NAME='%2';").arg(Ent->database()).arg(TableName);
    QString idxSql = QString("SHOW INDEXES FROM `%1`.`%2` WHERE Key_name != 'PRIMARY';").arg(Ent->database()).arg(TableName);
    QString fokSql = QString("SELECT * FROM `information_schema`.`KEY_COLUMN_USAGE` WHERE TABLE_SCHEMA='%1' AND TABLE_NAME='%2' AND CONSTRAINT_NAME != 'PRIMARY';").arg(Ent->database()).arg(TableName);
    QString colName;    
    QVariant value;
    Data = QJsonObject();
    if (Con->runSql(tabSql, &qry, &err))
    {
        if (qry.next())
        {
            for (int i=0;i<qry.record().count();i++)
            {
                colName = qry.record().fieldName(i);
                value = qry.value(colName);
                Data[colName] = value.toString();
            }
        }
        if (Con->runSql(colSql, &qry, &err))
        {
            QJsonObject cols;
            while (qry.next())
            {
                QJsonObject colObj;
                QString columnName;
                for (int i=0;i<qry.record().count();i++)
                {
                    colName = qry.record().fieldName(i);
                    value = qry.value(colName);
                    colObj[colName] = value.toString();
                    if (colName == "COLUMN_NAME")
                        columnName = value.toString();
                    if (colName == "IS_NULLABLE")
                        colObj[".NotNull"] = (value.toString().trimmed().toUpper() == "NO");
                    if (colName == "COLUMN_KEY")
                    {
                        colObj[".Unique"] = value.toString().trimmed().toUpper().contains("UNI");
                        colObj[".PrimaryKey"] = value.toString().trimmed().toUpper().contains("PRI");
                    }
                    if (colName == "EXTRA")
                        colObj[".AutoIncrement"] = value.toString().trimmed().toUpper().contains("AUTO_INCREMENT");
                }
                cols[columnName] = colObj;
            }
            Data[".columns"] = cols;
        }
        if (Con->runSql(idxSql, &qry, &err))
        {
            QJsonObject idxs;
            while(qry.next())
            {
                QJsonObject indx;
                QString indxName;
                for (int i=0;i<qry.record().count();i++)
                {
                    colName = qry.record().fieldName(i);
                    value = qry.value(colName);
                    if (colName == "Column_name")
                        indxName = value.toString();
                    else if (colName == "Index_type")
                        indx["Type"] = value.toString();
                    else if (colName == "Key_name")
                        indx["Name"] = value.toString();
                    else if (colName == "Index_type")
                        indx["Type"] = value.toString();
                    else if (colName == "Comment")
                        indx["Comment"] = value.toString();
                    else if (colName == "Index_commnent")
                        indx["IndexComment"] = value.toString();
                }
                idxs[indxName] = indx;
            }
            Data[".indexes"] = idxs;
        }
        if (Con->runSql(fokSql, &qry, &err))
        {
            QJsonObject foks;
            while(qry.next())
            {
                QJsonObject fok;
                QString fokName;
                for (int i=0;i<qry.record().count();i++)
                {
                    colName = qry.record().fieldName(i);
                    value = qry.value(colName);
                    if (colName == "CONSTRAINT_NAME")
                        fokName = value.toString();
                    else if (colName == "REFERENCED_TABLE_SCHEMA")
                        fok["Database"] = value.toString();
                    else if (colName == "REFERENCED_TABLE_NAME")
                        fok["Table"] = value.toString();
                    else if (colName == "REFERENCED_COLUMN_NAME")
                        fok["Column"] = value.toString();
                }
                foks[fokName] = fok;
            }
            Data[".constraints"] = foks;
        }
    }
    OriginData = QJsonObject(Data);
    //QString errStr = err.databaseText();
}

void GuiTable::showColumnData(QString cname)
{
    if (cname.isEmpty())
    {
        errMsg(tr("Column definition is empty"));
        return;
    }
    QJsonObject cols, col, indxs;
    cols = Data[".columns"].toObject();
    col = cols[cname].toObject();
    ui->ColumnName->setText(cname);
    ui->ColumnType->setCurrentText(col["COLUMN_TYPE"].toString());
    ui->ColumnNotNull->setChecked(col[".NotNull"].toBool());
    ui->ColumnUnique->setChecked(col[".Unique"].toBool());
    ui->ColumnPrimaryKey->setChecked(col[".PrimaryKey"].toBool());
    ui->ColumnAutoIncrement->setChecked(col[".AutoIncrement"].toBool());
    ui->ColumnDefaultValue->setText(col["COLUMN_DEFAULT"].toString());
    ui->ColumnComment->setText(col["COLUMN_COMMENT"].toString());
    ui->ColumnCollation->setCurrentText(col["COLLATION_NAME"].toString());
    ui->ColumnDelete->setEnabled(true);
    indxs = Data[".indexes"].toObject();
    ui->ColumnIndex->setChecked(indxs.contains(cname));
    //ui->ColumnSet->setDisabled(true);
}
void GuiTable::showFokData(QString fokname)
{

}
void GuiTable::fillEngines()
{
    QSqlDatabase db = QSqlDatabase::database(Ent->connection());
    QSqlQuery qry(db);
    QSqlError err;
    Con->runSql(QString("USE %1;").arg(Ent->database()), &qry, &err);
    if (Con->runSql("SHOW ENGINES", &qry, &err))
    {
        ui->TableEngine->clear();
        QString engine;
        while (qry.next())
        {
            engine = qry.value("Engine").toString();
            ui->TableEngine->addItem(engine);
        }
    }
    for (int i=0;i<Settings::Engines.count();i++)
        ui->TableEngine->addItem(Settings::Engines.at(i).toString());
}
void GuiTable::fillColumnTypes()
{
    QJsonArray types = Settings::ColumnTypes;
    QString type;
    ui->ColumnType->addItem("");
    for (int i=0;i<types.count();i++)
    {
        type = types.at(i).toString();
        ui->ColumnType->addItem(type);
    }
}
void GuiTable::fillCollations()
{
    QSqlDatabase db = QSqlDatabase::database(Ent->connection());
    QSqlQuery qry(db);
    QSqlError err;
    Con->runSql(QString("USE %1;").arg(Ent->database()), &qry, &err);
    if (Con->runSql("SHOW COLLATION", &qry, &err))
    {
        ui->TableCollation->clear();
        ui->ColumnCollation->clear();
        QString colt;
        while (qry.next())
        {
            colt = qry.value(0).toString().trimmed();
            ui->TableCollation->addItem(colt);
            ui->ColumnCollation->addItem(colt);
        }
    }
    for (int i=0;i<Settings::Engines.count();i++)
        ui->TableEngine->addItem(Settings::Engines.at(i).toString());
}
void GuiTable::refresh()
{
    Alter = false;
    Create = false;

    if (TableName.isEmpty())
    {
        Create = true;
    }
    else
    {
        Alter = true;
        makeTableData();
    }

    fillEngines();
    fillColumnTypes();
    fillCollations();
    showData();
    if (Create)
    {
        newColumnSlot();
        ui->TableCollation->setCurrentText(Con->Defaults["Collation"].toString());
        ui->TableEngine->setCurrentText(Con->Defaults["Engine"].toString());
    }
    if (Alter)
    {
        if (ui->Columns->count() == 0)
            newColumnSlot();
        else
        {
            ui->Columns->setCurrentRow(0);
            columnActivatedSlot(ui->Columns->currentItem());
        }
    }
}
GuiTable::GuiTable(DBConnectionPtr con, DbEntityPtr ent, QString name, QJsonObject data, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GuiTable)
{
    ui->setupUi(this);
    QList<int> sizes;
    sizes.append(250);
    sizes.append(750);
    ui->splitter->setSizes(sizes);
    ui->splitter_2->setSizes(sizes);
    //ui->Msg->setHidden(true);
    ui->tabWidget->setCurrentIndex(0);
    Con = con;
    Ent = ent;
    Data = data;
    TableName = name;

    connect(ui->ColumnNew, SIGNAL(released()), this, SLOT(newColumnSlot()));
    connect(ui->FokNew, SIGNAL(released()), this, SLOT(newFokSlot()));
    connect(ui->ColumnDelete, SIGNAL(released()), this, SLOT(deleteColumnSlot()));
    connect(ui->FokDelete, SIGNAL(released()), this, SLOT(deleteFokSlot()));
    connect(ui->ColumnSet, SIGNAL(released()), this, SLOT(setColumnSlot()));
    connect(ui->FokSet, SIGNAL(released()), this, SLOT(setFokSlot()));
    connect(ui->Apply, SIGNAL(released()), this, SLOT(applySlot()));
    connect(ui->SaveAsTemplate, SIGNAL(released()), this, SLOT(saveAsTemplateSlot()));
    connect(ui->LoadFromTemplate, SIGNAL(released()), this, SLOT(loadFromTemplateSlot()));
    connect(ui->RemoveTemplate, SIGNAL(released()), this, SLOT(removeTemplateSlot()));
    connect(ui->Columns, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(columnActivatedSlot(QListWidgetItem*)));
    connect(ui->ForeignKeys, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(fokActivatedSlot(QListWidgetItem*)));
    connect(ui->ShowSql, SIGNAL(released()), this, SLOT(showSqlCmdSlot()));
    connect(ui->Refresh, SIGNAL(released()), this, SLOT(refresh()));

    refresh();    
}

GuiTable::~GuiTable()
{
    delete ui;
}
