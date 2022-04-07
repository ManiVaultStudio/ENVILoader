#include "ENVILoaderModel.h"
#include "ENVILoaderPlugin.h"
#include "ENVILoader.h"

#include "Application.h"

#include <QtCore>
#include <QDebug>
#include <QDirIterator>
#include <QPainter>
#include <QIcon>

using namespace hdps;

ENVILoaderModel::ENVILoaderModel(ENVILoaderPlugin* ENVILoaderPlugin) :
    QAbstractItemModel(),
    _ENVILoaderPlugin(ENVILoaderPlugin),
    _selectionModel(this),
    _persistData(true)
{
}

ENVILoaderModel::~ENVILoaderModel()
{
}

int ENVILoaderModel::rowCount(const QModelIndex& parent /* = QModelIndex() */) const {
    return 0;
}

int ENVILoaderModel::columnCount(const QModelIndex& parent) const {
    return 0;
}

/**
 * Returns model data for the given index
 * @param index Index
 * @param role The data role
 */
QVariant ENVILoaderModel::data(const QModelIndex& index, int role /* = Qt::DisplayRole */) const {
    return "0";
}

/**
 * Sets the data value for the given model index and data role
 * @param index Model index
 * @param value Data value in variant form
 * @param role Data role
 * @return Whether the data was properly set or not
 */
bool ENVILoaderModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    return true;
}

/**
 * Returns the header data for the given section, orientation and data role
 * @param section Model section
 * @param orientation Orientation (e.g. horizontal or vertical)
 * @param role Data role
 * @return Header data in variant form
 */
QVariant ENVILoaderModel::headerData(int section, Qt::Orientation orientation, int role) const {
    return "0";
}

/**
 * Returns the item flags for the given model index
 * @param index Model index
 * @return Item flags for the index
 */
Qt::ItemFlags ENVILoaderModel::flags(const QModelIndex& index) const {
    auto flags = static_cast<Qt::ItemFlags>(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);

    return flags;
}

/**
 * Returns the model for the given row and column and parent model index
 * @param row Row
 * @param column Column
 * @param parent Parent model index
 * @return Model index
 */
QModelIndex ENVILoaderModel::index(int row, int column, const QModelIndex& parent) const {
    return QModelIndex();
}

/**
 * Returns the parent model index of the given model index
 * @param index Model index
 * @return Parent model index
 */
QModelIndex ENVILoaderModel::parent(const QModelIndex& index) const {
    return QModelIndex();
}

bool ENVILoaderModel::load(ENVILoaderPlugin* ENVILoaderPlugin, float ratio, int filter, QString fileName) {
    
    QString name = fileName.mid(fileName.lastIndexOf("/") + 1);
    name.chop(4);

    auto _core = ENVILoaderPlugin->getCore();
    
    ENVILoader* loader = new ENVILoader(_core, name);
    loader->loadFromFile(fileName.toStdString(), ratio, filter);

    return true;
}