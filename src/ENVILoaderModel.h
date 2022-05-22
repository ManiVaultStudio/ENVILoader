#pragma once

#include "ENVILoader.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QItemSelectionModel>

class ENVILoaderPlugin;

class ENVILoaderModel : public QAbstractItemModel
{
public: // Construction/destruction

    /**
     * Constructor
     * @param ENVILoaderPlugin Pointer to ENVI loader plugin
     */
    ENVILoaderModel(ENVILoaderPlugin* ENVILoaderPlugin);

    /** Default destructor */
    ~ENVILoaderModel();

public: // Inherited MVC

/**
 * @param parent Parent index
 */
    int rowCount(const QModelIndex& parent /* = QModelIndex() */) const override;

    /**
     * Returns the number of columns in the model given the parent model index
     * @param parent Parent model index
     * @return Number of columns in the model given the parent model index
     */
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * Returns model data for the given index
     * @param index Index
     * @param role The data role
     */
    QVariant data(const QModelIndex& index, int role /* = Qt::DisplayRole */) const override;

    /**
     * Sets the data value for the given model index and data role
     * @param index Model index
     * @param value Data value in variant form
     * @param role Data role
     * @return Whether the data was properly set or not
     */
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    /**
     * Returns the header data for the given section, orientation and data role
     * @param section Model section
     * @param orientation Orientation (e.g. horizontal or vertical)
     * @param role Data role
     * @return Header data in variant form
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /**
     * Returns the item flags for the given model index
     * @param index Model index
     * @return Item flags for the index
     */
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    /**
     * Returns the model for the given row and column and parent model index
     * @param row Row
     * @param column Column
     * @param parent Parent model index
     * @return Model index
     */
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    /**
     * Returns the parent model index of the given model index
     * @param index Model index
     * @return Parent model index
     */
    QModelIndex parent(const QModelIndex& index) const override;

public:
    std::pair<size_t, size_t> init(ENVILoaderPlugin* ENVILoaderPlugin, QString fileName);

    bool load(float ratio, int filter, bool invertYAxis);

private:
    ENVILoaderPlugin* _ENVILoaderPlugin;     /** ENVI loader plugin instance */
    ENVILoader* _loader;
    QItemSelectionModel     _selectionModel;        /** Selection model */
    bool                    _persistData;           /** Whether updates to the model data are persisted */
};