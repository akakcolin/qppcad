#include <iostream>
#include <csignal>
#include <QApplication>
#include <QStyleFactory>
#include <QFontDatabase>
#include <qppcad/ui/main_window.hpp>
#include <qppcad/core/app_state.hpp>
#include <qppcad/core/hotkey_manager.hpp>
#include <QMainWindow>
#include <QTextStream>
#include <QCommandLineParser>

using namespace qpp;
using namespace qpp::cad;

void signal_handler(int signal) {

  std::cout << "SIGNAL RECEIVED " << signal << std::endl;
  app_state_t::get_inst()->save_settings();

}

int main (int argc, char **argv) {

  std::ios_base::sync_with_stdio(false);

  QCoreApplication::setOrganizationName("igc");
  QCoreApplication::setOrganizationDomain("igc.irk.ru");
  QCoreApplication::setApplicationName("qppcad");
  QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QApplication app(argc, argv);

  QCommandLineParser parser;
  parser.setApplicationDescription("qpp::cad");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("file", "The file to open.");

  QCommandLineOption target_fmt_option(QStringList() << "f" << "format",
                                       QCoreApplication::translate("main", "Force file format"),
                                       QCoreApplication::translate("main", "file_format"));
  parser.addOption(target_fmt_option);

  parser.process(QCoreApplication::arguments());
  const QStringList args = parser.positionalArguments();

  app_state_t::init_inst();

  app_state_t *astate = app_state_t::get_inst();

  astate->tlog("@GIT_REVISION={}, @BUILD_DATE={}",
               build_info_t::get_git_version(), build_info_t::get_git_version());

  std::ifstream test_in_dev_env("../data/refs/laf3_p3.vasp");
  bool under_dev_env = test_in_dev_env.good();

  astate->init_managers();
  astate->ws_mgr->init_ws_item_bhv_mgr();
  astate->load_settings();

  std::signal(SIGSEGV, signal_handler);

  astate->init_fixtures();

  if (under_dev_env) {
      astate->ws_mgr->init_default();
      //astate->m_show_debug_frame_stats = true;
    }

  if (!args.empty()) {

      std::string file_format = "";

      if (parser.isSet(target_fmt_option))
        file_format = parser.value(target_fmt_option).toStdString();

      for (auto &rec : args) {
          astate->tlog("@DEBUG: passed to load_from_file_autodeduce, path={}, ff={}",
                       rec.toStdString(), file_format.empty() ? "NONE" : file_format);
          astate->ws_mgr->load_from_file_autodeduce(rec.toStdString(), file_format);
        }

    } else {

      if (parser.isSet(target_fmt_option)) {
          astate->tlog("ERROR: Invalid input");
          return 0;
        }

    }

  QSurfaceFormat format;
  format.setDepthBufferSize(24);

  //format.setStencilBufferSize(8);
  format.setSamples(astate->m_num_samples);
  format.setVersion(3, 3);
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);

  qApp->setStyle(QStyleFactory::create("Fusion"));
  astate->init_styles();
  //qApp->setPalette(astate->m_app_palette);

  std::vector<int> fnt_id = {
    QFontDatabase::addApplicationFont("://fonts/Heebo/Heebo-Light.ttf"),
    QFontDatabase::addApplicationFont("://fonts/Heebo/Heebo-Regular.ttf"),
    QFontDatabase::addApplicationFont("://fonts/Heebo/Heebo-Bold.ttf"),
  };

  QString family = QFontDatabase::applicationFontFamilies(fnt_id[0]).at(0);
  astate->m_font_name = family;
  astate->tlog("Font used : {}", astate->m_font_name.toStdString());

  QIcon icon("://images/icon.svg");
  app.setWindowIcon(icon);

  QFile file("://style.qss");
  file.open(QFile::ReadOnly);
  QString style_sheet = QLatin1String(file.readAll());
  app.setStyleSheet(style_sheet);

  main_window_t w;
  astate->hotkey_mgr->m_main_window = &w;
  astate->hotkey_mgr->bootstrap_from_restore_info();

  w.rebuild_recent_files_menu();
  astate->ws_mgr->m_bhv_mgr->cache_obj_insp_widgets();
  w.showMaximized();

  int ret_code = app.exec();

  app_state_t::get_inst()->save_settings();

  return ret_code;

}
