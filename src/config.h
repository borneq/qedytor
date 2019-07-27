#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include "configitem.h"

namespace qedytor {
class Config;

enum class SortBy {original, lastEditTime, closingTime, path, name};

struct ConfigFile: public ConfigItem
{
    int row = 0;
    int col = 0;
    qint64 lastEditTime = 0;
    qint64 closingTime = 0;
    QString syntax;
    bool wordWrap = false;
    ConfigFile();
    void load(QJsonValue v);
    QJsonValue save() const;
    QVariant getData(int col) override;
};

struct ConfigEditor: public ConfigItem
{
    void load(QJsonValue json_obj);
    QJsonValue save() const;
    QVariant getData(int col) override;
};


class Config
{
    const int maxMRU = 20;
    friend struct ConfigPosition;
    friend struct ConfigFile;
    friend struct ConfigEditor;
    const QString configName = "QEdytor.json";
    QString findPath();
    QString configPath;
    bool dirty = false;
    static QJsonObject loadJson(QString fileName);
    static void saveJson(QJsonObject json_obj, QString fileName);
    static QString toString(QJsonObject json_obj, QString key);
    static void toInt(QJsonObject json_obj, QString key, int &n);
    QJsonObject json_obj;
    void json_to_list(const QJsonObject &json_obj, QString valueName, QList<ConfigItem*> &configList);
    void list_to_json(const QList<ConfigItem*> &configList, QString valueName, QJsonObject &json_obj);
    void replaceInHandy(int pos, ConfigFile *configFile);
    void replaceInMru(int pos, ConfigFile *configFile);
    int findOldestMru();
    void removeOldestMru();
    void addToMru(ConfigFile *configFile);
public:
    QStringList configEditorHeaders;
    QStringList configFilesHeaders;
    int findInHandy(QString &path);
    int findInMru(QString &path);
    void removeMru(int n);
    ConfigFile *cfFindInHandyOrMru(QString &path);
    Config();
    ~Config();
    void readFromFile();
    void saveToFile();
    void setDirty();
    void addClosedFile(ConfigFile *configFile);
    QString pathToSyntax;
    QString pathToThemes;
    QString getConfigPath() { return configPath;}
    QByteArray geometry;
    QList<ConfigItem*> handy;
    QList<ConfigItem*> mru;
    QList<ConfigItem*> otherEditors;
};
}
#endif // CONFIG_H
