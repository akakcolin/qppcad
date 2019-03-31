#include <qppcad/ws_item/geom_view/xgeom_table_model.hpp>

using namespace qpp;
using namespace qpp::cad;


int xgeom_table_model_t::rowCount(const QModelIndex &parent) const {

  if (m_xgeom) {
      return m_xgeom->nat();
    } else {
      return 0;
    }

}

int xgeom_table_model_t::columnCount(const QModelIndex &parent) const {

  if (m_xgeom) {
      return m_xgeom->nfields();
    } else {
      return 0;
    }

}

QVariant xgeom_table_model_t::data(const QModelIndex &index, int role) const {

  if (!m_xgeom) return QVariant();

  if (!index.isValid()) return QVariant();

  if (role == Qt::TextAlignmentRole) {
      return Qt::AlignCenter;
    }

  int xfield_index = index.column();
  int atom_id = index.row();
  basic_types field_type = m_xgeom->field_type(xfield_index);

  if (role == Qt::DisplayRole) {

     switch (field_type) {
       case basic_types::type_bool : {
           return QVariant();
           break;
         }
       case basic_types::type_double : {
           double field_val = m_xgeom->xfield<double>(xfield_index, atom_id);
           return QVariant(field_val);
           break;
         }
       case basic_types::type_float : {
           float field_val = m_xgeom->xfield<float>(xfield_index, atom_id);
           return QVariant(field_val);
           break;
         }
       case basic_types::type_real : {
           float field_val = m_xgeom->xfield<float>(xfield_index, atom_id);
           return QVariant(field_val);
           break;
         }
       case basic_types::type_int : {
           return QString("%1").arg(m_xgeom->xfield<int>(xfield_index, atom_id));
           break;
         }
       case basic_types::type_string : {
           QString field_val = QString::fromStdString(
                              m_xgeom->xfield<std::string>(xfield_index, atom_id));
           return field_val;
           break;
         }
       }

     return QVariant();

    }

 if (field_type == basic_types::type_bool && role == Qt::CheckStateRole) {
     if (m_xgeom->xfield<bool>(xfield_index, atom_id)) return Qt::Checked;
     else return Qt::Unchecked;
   }

  return QVariant();

}

QVariant xgeom_table_model_t::headerData(int section,
                                         Qt::Orientation orientation, int role) const {

  if (!m_xgeom) return QVariant();

  if (role == Qt::DisplayRole) {
      if (orientation == Qt::Horizontal) {
          std::string field_name = m_xgeom->field_name(section);
          return QVariant(QString::fromStdString(field_name));
        }
    } else {
        return QVariant();
    }

  return QVariant();

}

Qt::ItemFlags xgeom_table_model_t::flags(const QModelIndex &index) const {

  Qt::ItemFlags flags =  QAbstractItemModel::flags(index) |
                         Qt::ItemFlag::ItemIsEnabled |
                         Qt::ItemFlag::ItemIsEditable;

  if (!index.isValid())
        return Qt::ItemIsEnabled;

  if (m_xgeom) {
      int xfield_index = index.column();
      int atom_id = index.row();
      basic_types field_type = m_xgeom->field_type(xfield_index);
      if (field_type == basic_types::type_bool)
        flags = flags | Qt::ItemFlag::ItemIsUserCheckable;
    }

  return flags;

}

bool xgeom_table_model_t::setData(const QModelIndex &index, const QVariant &value, int role) {

  return QAbstractTableModel::setData(index, value, role);

}

void xgeom_table_model_t::bind(xgeometry<float, periodic_cell<float> > *_xgeom) {

  m_xgeom = _xgeom;

}

void xgeom_table_model_t::unbind() {

  m_xgeom = nullptr;

}
