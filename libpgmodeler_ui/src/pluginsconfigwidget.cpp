#include "pluginsconfigwidget.h"

PluginsConfigWidget::PluginsConfigWidget(QWidget *parent) : QWidget(parent)
{
 setupUi(this);

 QGridLayout *grid=new QGridLayout(loaded_plugins_gb);
 QDir dir=QDir(GlobalAttributes::PLUGINS_DIR);

 root_dir_edt->setText(dir.absolutePath());

 plugins_tab=new TabelaObjetosWidget(TabelaObjetosWidget::BTN_EDITAR_ITEM, false, this);
 plugins_tab->definirNumColunas(3);
 plugins_tab->definirRotuloCabecalho(trUtf8("Plugin"),0);
 plugins_tab->definirIconeCabecalho(QPixmap(":/icones/icones/plugins.png"),0);
 plugins_tab->definirRotuloCabecalho(trUtf8("Version"),1);
 plugins_tab->definirRotuloCabecalho(trUtf8("Library"),2);

 connect(plugins_tab, SIGNAL(s_linhaEditada(int)), this, SLOT(showPluginInfo(int)));
 connect(open_fm_tb, SIGNAL(clicked(void)), this, SLOT(openRootPluginDiretory(void)));

 grid->setContentsMargins(2,2,2,2);
 grid->addWidget(plugins_tab,0,0,1,1);
 loaded_plugins_gb->setLayout(grid);
}

PluginsConfigWidget::~PluginsConfigWidget(void)
{
 while(!plugins.empty())
 {
  delete(plugins.back());
  plugins.pop_back();
 }
}

void PluginsConfigWidget::openRootPluginDiretory(void)
{
 QDesktopServices::openUrl(QUrl("file:///" + root_dir_edt->text()));
}

void PluginsConfigWidget::showPluginInfo(int idx)
{
 plugins[idx]->showPluginInfo();
}

void PluginsConfigWidget::loadPlugins(void)
{
 vector<Exception> errors;
 QString lib, plugin_name,
         dir_plugins=GlobalAttributes::PLUGINS_DIR +
                     GlobalAttributes::DIR_SEPARATOR;
 QPluginLoader plugin_loader;
 QStringList dir_list;
 PgModelerPlugin *plugin=NULL;
 QAction *plugin_action=NULL;
 QPixmap icon;
 QFileInfo fi;

 //The plugin loader must resolve all symbols otherwise return an error if some symbol is missing on library
 plugin_loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);

 if(GlobalAttributes::PLUGINS_DIR.isEmpty())
  dir_plugins="." + GlobalAttributes::DIR_SEPARATOR;

 /* Configures an QDir instance to list only directories on the plugins/ subdir.
    If the user does not put the plugin in it's directory the file is ignored  */
 dir_list=QDir(dir_plugins, "*", QDir::Name, QDir::AllDirs | QDir::NoDotAndDotDot).entryList();

 while(!dir_list.isEmpty())
 {
  plugin_name=dir_list.front();

  /* Configures the basic path to the library on the form:

     [PLUGINS ROOT DIR]/[PLUGIN NAME]/lib[PLUGIN NAME].[EXTENSION] */
  #ifdef Q_OS_WIN
   lib=dir_plugins + plugin_name +
       GlobalAttributes::DIR_SEPARATOR  +
       plugin_name + QString(".dll");
  #else
    #ifdef Q_OS_MAC
     lib=dir_plugins + plugin_name +
         GlobalAttributes::DIR_SEPARATOR  +
         QString("lib") + plugin_name + QString(".dylib");
    #else
     lib=dir_plugins + plugin_name +
         GlobalAttributes::DIR_SEPARATOR  +
         QString("lib") + plugin_name + QString(".so");
    #endif
  #endif

  //Try to load the library
  plugin_loader.setFileName(lib);
  if(plugin_loader.load())
  {
   fi.setFile(lib);

   //Inserts the loaded plugin on the vector
   plugin=qobject_cast<PgModelerPlugin *>(plugin_loader.instance());
   plugins.push_back(plugin);

   //Configures the action related to plugin
   plugin_action=new QAction(this);
   plugin_action->setText(plugin->getPluginTitle());
   plugin_action->setData(QVariant::fromValue<void *>(reinterpret_cast<void *>(plugin)));

   icon.load(dir_plugins + plugin_name +
                 GlobalAttributes::DIR_SEPARATOR  +
                 plugin_name + QString(".png"));
   plugin_action->setIcon(icon);

   plugins_actions.push_back(plugin_action);
   plugins_tab->adicionarLinha();
   plugins_tab->definirTextoCelula(plugin->getPluginTitle(), plugins_tab->obterNumLinhas()-1, 0);
   plugins_tab->definirTextoCelula(plugin->getPluginVersion(), plugins_tab->obterNumLinhas()-1, 1);
   plugins_tab->definirTextoCelula(fi.fileName(), plugins_tab->obterNumLinhas()-1, 2);
  }
  else
  {
   errors.push_back(Exception(Exception::getErrorMessage(ERR_PLUGIN_NOT_LOADED)
                               .arg(QString::fromUtf8(dir_list.front()))
                               .arg(QString::fromUtf8(lib))
                               .arg(plugin_loader.errorString()),
                               ERR_PLUGIN_NOT_LOADED, __PRETTY_FUNCTION__,__FILE__,__LINE__));
  }
  dir_list.pop_front();
  plugins_tab->limparSelecao();
 }

 if(!errors.empty())
  throw Exception(ERR_PLUGINS_NOT_LOADED,__PRETTY_FUNCTION__,__FILE__,__LINE__, errors);
}

void PluginsConfigWidget::installPluginsActions(QToolBar *toolbar, QMenu *menu, QObject *recv, const char *slot)
{
 if((toolbar || menu) && slot)
 {
  vector<QAction *>::iterator itr=plugins_actions.begin();

  while(itr!=plugins_actions.end())
  {
   if(toolbar)
    toolbar->addAction(*itr);

   if(menu)
    menu->addAction(*itr);

   connect(*itr, SIGNAL(triggered(void)), recv, slot);
   itr++;
  }
 }
}