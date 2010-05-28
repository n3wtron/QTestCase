#ifndef SELECTIONTABLEVIEW_H
#define SELECTIONTABLEVIEW_H

#include <QTableView>
#include <QModelIndexList>

class SelectionTableView : public QTableView
{
Q_OBJECT
public:
    explicit SelectionTableView(QWidget *parent = 0);
    virtual QModelIndexList selectedIndexes () const;
signals:

public slots:

};

#endif // SELECTIONTABLEVIEW_H
