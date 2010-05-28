#include "selectiontableview.h"

SelectionTableView::SelectionTableView(QWidget *parent) :
    QTableView(parent)
{
}

QModelIndexList SelectionTableView::selectedIndexes () const{
    return QTableView::selectedIndexes();
}
