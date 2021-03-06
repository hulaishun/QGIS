/**
 * Layer tree group node serves as a container for layers and further groups.
 *
 * Group names do not need to be unique within one tree nor within one parent.
 *
 * @note added in 2.4
 */
class QgsLayerTreeGroup : QgsLayerTreeNode
{
%TypeHeaderCode
#include <qgslayertreegroup.h>
%End

  public:
    QgsLayerTreeGroup( const QString& name = QString(), Qt::CheckState checked = Qt::Checked );

    //! Get group's name
    QString name() const;
    //! Set group's name
    void setName( const QString& n );

    //! Insert a new group node with given name at specified position. Newly created node is owned by this group.
    QgsLayerTreeGroup* insertGroup( int index, const QString& name );
    //! Append a new group node with given name. Newly created node is owned by this group.
    QgsLayerTreeGroup* addGroup( const QString& name );
    //! Insert a new layer node for given map layer at specified position. Newly created node is owned by this group.
    QgsLayerTreeLayer* insertLayer( int index, QgsMapLayer* layer );
    //! Append a new layer node for given map layer. Newly created node is owned by this group.
    QgsLayerTreeLayer* addLayer( QgsMapLayer* layer );

    //! Insert existing nodes at specified position. The nodes must not have a parent yet. The nodes will be owned by this group.
    void insertChildNodes( int index, const QList<QgsLayerTreeNode*>& nodes /Transfer/ );
    //! Insert existing node at specified position. The node must not have a parent yet. The node will be owned by this group.
    void insertChildNode( int index, QgsLayerTreeNode* node /Transfer/ );
    //! Append an existing node. The node must not have a parent yet. The node will be owned by this group.
    void addChildNode( QgsLayerTreeNode* node /Transfer/ );

    //! Remove a child node from this group. The node will be deleted.
    void removeChildNode( QgsLayerTreeNode* node );
    //! Remove map layer's node from this group. The node will be deleted.
    void removeLayer( QgsMapLayer* layer );
    //! Remove child nodes from index "from". The nodes will be deleted.
    void removeChildren( int from, int count );
    //! Remove all child group nodes without layers. The groupnodes will be deleted.
    void removeChildrenGroupWithoutLayers();
    //! Remove all child nodes. The nodes will be deleted.
    void removeAllChildren();

    //! Find layer node representing the map layer. Searches recursively the whole sub-tree.
    //! @note added in 3.0
    QgsLayerTreeLayer* findLayer( QgsMapLayer* layer ) const;
    //! Find layer node representing the map layer specified by its ID. Searches recursively the whole sub-tree.
    QgsLayerTreeLayer* findLayer( const QString& layerId );
    //! Find all layer nodes. Searches recursively the whole sub-tree.
    QList<QgsLayerTreeLayer*> findLayers() const;
    //! Find layer IDs used in all layer nodes. Searches recursively the whole sub-tree.
    QStringList findLayerIds() const;
    //! Find group node with specified name. Searches recursively the whole sub-tree.
    QgsLayerTreeGroup* findGroup( const QString& name );

    //! Read group (tree) from XML element <layer-tree-group> and return the newly created group (or null on error).
    //! Does not resolve textual references to layers. Call resolveReferences() afterwards to do it.
    static QgsLayerTreeGroup* readXml( QDomElement& element ) /Factory/;
    //! Read group (tree) from XML element <layer-tree-group> and return the newly created group (or null on error).
    //! Also resolves textual references to layers from the project (calls resolveReferences() internally).
    //! @note added in 3.0
    static QgsLayerTreeGroup* readXml( QDomElement& element, const QgsProject* project ) /Factory/;

    //! Write group (tree) as XML element <layer-tree-group> and add it to the given parent element
    virtual void writeXml( QDomElement& parentElement );
    //! Read children from XML and append them to the group.
    //! Does not resolve textual references to layers. Call resolveReferences() afterwards to do it.
    void readChildrenFromXml( QDomElement& element );

    //! Return text representation of the tree. For debugging purposes only.
    virtual QString dump() const;

    //! Return a clone of the group. The children are cloned too.
    virtual QgsLayerTreeGroup* clone() const /Factory/;

    //! Calls resolveReferences() on child tree nodes
    //! @note added in 3.0
    virtual void resolveReferences( const QgsProject* project );

    //! Return whether the group is mutually exclusive (only one child can be checked at a time)
    //! @note added in 2.12
    bool isMutuallyExclusive() const;
    //! Set whether the group is mutually exclusive (only one child can be checked at a time).
    //! The initial child index determines which child should be initially checked. The default value
    //! of -1 will determine automatically (either first one currently checked or none)
    //! @note added in 2.12
    void setIsMutuallyExclusive( bool enabled, int initialChildIndex = -1 );

  protected slots:
    void nodeVisibilityChanged( QgsLayerTreeNode* node );

  protected:

    //! Set check state of children - if mutually exclusive
    void updateChildVisibilityMutuallyExclusive();

  private:
    QgsLayerTreeGroup( const QgsLayerTreeGroup& other );
};
