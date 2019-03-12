#include "config.h"
#include "edytorexception.h"
#include <QDir>
#include <QJsonArray>
#include <QDebug>

using namespace qedytor;

Config::Config()
{
    configEditorHeaders = QStringList{"Path", "Name"};
    configFilesHeaders = QStringList{"Path","Row","Col","lastEditTime","closingTime","Syntax","wordWrap"};
}

Config::~Config()
{
    for (ConfigItem* cf: handy)
        delete cf;
    for (ConfigItem* cf: mru)
        delete cf;
    for (ConfigItem* ce: otherEditors)
        delete ce;
}

QString Config::findPath()
{
    QDir homeDir(QDir::homePath());
    QDir homeConfigDir = homeDir.filePath(".config");
    QString homeConfigPath = homeConfigDir.filePath(configName);
    configPath = homeConfigPath;
    QFileInfo fiHome(homeConfigPath);
    if (fiHome.exists())
    {
        if (!fiHome.isFile())
            throw EdytorException(homeConfigPath+ " is not a file");
        if (!fiHome.isReadable())
            throw EdytorException(homeConfigPath+ " is not readable");
        return homeConfigPath;
    }
    else throw EdytorException("I can't find "+configName+" in (home) dir\n"+homeConfigDir.absolutePath());
}

QJsonObject Config::loadJson(QString fileName)
{
    QFile jsonFile(fileName);
    jsonFile.open(QFile::ReadOnly);
    return QJsonDocument().fromJson(jsonFile.readAll()).object();
}

void Config::saveJson(QJsonObject json_obj, QString fileName)
{
    QJsonDocument json_doc(json_obj);
    QFileInfo fi(fileName);
    QDir dir = fi.dir();
    dir.mkpath(dir.path());
    QFile jsonFile(fileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(json_doc.toJson());
}

QString Config::toString(QJsonObject json_obj, QString key)
{
    QJsonObject::iterator it =json_obj.find(key);
    if (it==json_obj.constEnd())
        return "";
    else
        return it->toString();
}

/**
 * @brief Config::toInt
 * returns result by reference instead of result because if not
 * find in json must not change result
 * @param json_obj
 * @param key
 * @param n
 */

void Config::toInt(QJsonObject json_obj, QString key, int &n)
{
    QJsonObject::iterator it =json_obj.find(key);
    if (it!=json_obj.constEnd())
       n = it->toInt();
}

void Config::json_to_list(const QJsonObject &json_obj, QString valueName, QList<ConfigItem*> &configList)
{
    QJsonValue value = json_obj.value(valueName);
    QJsonArray array = value.toArray();
    foreach (const QJsonValue & v, array)
    {
        ConfigFile *cf = new ConfigFile;
        cf->load(v);
        configList.append(cf);
    }
}

void Config::list_to_json(const QList<ConfigItem*> &configList, QString valueName, QJsonObject &json_obj)
{
    QJsonArray array;
    foreach (const ConfigItem *cf, configList)
    {
        QJsonValue value = dynamic_cast<const ConfigFile*>(cf)->save();
        array.append(value);
    }
    json_obj.insert(valueName, array);
}


void Config::readFromFile()
{
    configPath = findPath();
    json_obj = loadJson(configPath);
    pathToSyntax = toString(json_obj, "pathToSyntax");
    pathToThemes = toString(json_obj, "pathToThemes");
    QString geometryHex = toString(json_obj, "geometry");
    geometry = QByteArray::fromHex(geometryHex.toLatin1());
    json_to_list(json_obj, "handyFiles", handy);
    json_to_list(json_obj, "mruFiles", mru);
    QJsonValue value = json_obj.value("otherEditors");
    QJsonArray array = value.toArray();
    foreach (const QJsonValue & v, array)
    {
        ConfigEditor *ce = new ConfigEditor;
        ce->load(v);
        otherEditors.append(ce);
    }
    dirty = false;
}

void Config::saveToFile()
{
    QJsonObject json_obj;
    json_obj.insert("pathToSyntax", pathToSyntax);
    json_obj.insert("pathToThemes", pathToThemes);
    QString geometryHex = geometry.toHex();
    json_obj.insert("geometry", geometryHex);
    list_to_json(handy, "handyFiles", json_obj);
    list_to_json(mru, "mruFiles", json_obj);

    QJsonArray array;
    foreach (const ConfigItem *ce, otherEditors)
    {
        QJsonValue value = dynamic_cast<const ConfigEditor*>(ce)->save();
        array.append(value);
    }
    json_obj.insert("otherEditors", array);
    saveJson(json_obj, configPath);
}

int Config::findInHandy(QString &path)
{
    for (int i=0; i<handy.size(); i++)
#ifdef _WIN32
        if (QString::compare(handy[i]->path, path, Qt::CaseInsensitive)==0)
#else
        if (handy[i]->path==path)
#endif
            return i;
    return -1;
}

int Config::findInMru(QString &path)
{
    for (int i=0; i<mru.size(); i++)
#ifdef _WIN32
        if (QString::compare(mru[i]->path, path, Qt::CaseInsensitive)==0)
#else
        if (mru[i]->path==path)
#endif
            return i;
    return -1;
}

ConfigFile *Config::cfFindInHandyOrMru(QString &path)
{
    if (path=="") return nullptr;
    int n = findInHandy(path);
    if (n>=0)
    {
        return dynamic_cast<ConfigFile*>(handy[n]);
    }
    else {
        int n = findInMru(path);
        if (n>=0)
            return dynamic_cast<ConfigFile*>(mru[n]);
    }
    return nullptr;
}

void Config::replaceInHandy(int pos, ConfigFile *configFile)
{
    delete handy[pos];
    handy[pos] = configFile;
}

void Config::replaceInMru(int pos, ConfigFile *configFile)
{
    delete mru[pos];
    mru[pos] = configFile;
}

int Config::findOldestMru()
{
    qint64 time = std::numeric_limits<qint64>::max();
    int oldestPos = -1;
    for (int i=0; i<mru.size(); i++)
    {
        if (dynamic_cast<ConfigFile*>(mru[i])->closingTime<time)
        {
            time = dynamic_cast<ConfigFile*>(mru[i])->closingTime;
            oldestPos = i;
        }
    }
    return oldestPos;
}

void Config::removeOldestMru()
{
    for (int i=mru.size()-1; i>=maxMRU; i--)
    {
        int n = findOldestMru();
        delete mru[n];
        mru.removeAt(n);
    }
}

void Config::addToMru(ConfigFile *configFile)
{
    mru.append(configFile);
}

void Config::addClosedFile(ConfigFile *configFile)
{
    int n = findInHandy(configFile->path);
    if (n>=0)
        replaceInHandy(n, configFile);
    else {
        n = findInMru(configFile->path);
        if (n>=0)
            replaceInMru(n, configFile);
        else {
            removeOldestMru();
            addToMru(configFile);
        }
    }
}

ConfigFile::ConfigFile()
{
    path = "";
    row = 1;
    col = 1;
    lastEditTime = 0;
    closingTime = 0;
    syntax = "";
    wordWrap = true;
}

void ConfigFile::load(QJsonValue v)
{
    QJsonObject json_obj = v.toObject();
    path = Config::toString(json_obj, "path");
    Config::toInt(json_obj, "row", row);
    Config::toInt(json_obj, "col", col);
    QString longStr = Config::toString(json_obj, "lastEditTime");
    lastEditTime = longStr.toLong();
    longStr = Config::toString(json_obj, "closingTime");
    closingTime = longStr.toLong();
    syntax = Config::toString(json_obj, "syntax");
    int wordWrapInt;
    Config::toInt(json_obj, "wordWrap", wordWrapInt);
    wordWrap = !!wordWrapInt;
}

QJsonValue ConfigFile::save() const
{
    QJsonObject json_obj;
    QString longStr;
    json_obj.insert("path", path);
    json_obj.insert("row", row);
    json_obj.insert("col", col);
    longStr.setNum(lastEditTime);
    json_obj.insert("lastEditTime", longStr);
    longStr.setNum(closingTime);
    json_obj.insert("closingTime", longStr);
    json_obj.insert("syntax", syntax);
    json_obj.insert("wordWrap", int(wordWrap));
    QJsonValue json_value(json_obj);
    return json_value;
}

QVariant ConfigFile::getData(int col)
{
    switch (col)
    {
    case 0: return path;
    case 1: return row;
    case 2: return col;
    case 3: return lastEditTime;
    case 4: return closingTime;
    case 5: return syntax;
    case 6: return wordWrap;
    default: return QVariant();
    }
}

QVariant ConfigEditor::getData(int col)
{
    switch (col)
    {
    case 0: return path;
    case 1: return name;
    default: return QVariant();
    }
}

void ConfigEditor::load(QJsonValue v)
{
    QJsonObject json_obj = v.toObject();
    name = Config::toString(json_obj, "name");
    path = Config::toString(json_obj, "path");
}

QJsonValue ConfigEditor::save() const
{
    QJsonObject json_obj;
    json_obj.insert("name", name);
    json_obj.insert("path", path);
    QJsonValue json_value(json_obj);
    return json_value;
}

void Config::setDirty()
{
    dirty = true;
}
