/*
  Q Light Controller
  sceneeditor.cpp

  Copyright (C) 2000, 2001, 2002 Heikki Junnila
                            2002 Stefan Krumm

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "app.h"
#include "doc.h"
#include "settings.h"
#include "sceneeditor.h"
#include "function.h"
#include "deviceclass.h"
#include "device.h"
#include "scene.h"
#include "consolechannel.h"
#include "listboxiditem.h"
#include "configkeys.h"

#include <qlistbox.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <stdlib.h>
#include <unistd.h>
#include <qpopupmenu.h>
#include <qtoolbutton.h>
#include <assert.h>

#include <iostream>
using namespace std;

extern App* _app;

const int KMenuActivate ( 1000 );
const int KMenuNew      ( 1001 );
const int KMenuStore    ( 1002 );
const int KMenuRemove   ( 1003 );
const int KMenuRename   ( 1004 );

static const QString KStatusStored        ( "Stored"      );
static const QString KStatusUnchanged     ( "Unchanged"   );
static const QString KStatusModified      ( "Modified"    );

static const QColor KStatusColorStored    ( 100, 255, 100 );
static const QColor KStatusColorUnchanged ( 255, 255, 255 );
static const QColor KStatusColorModified  ( 255, 100, 100 );

SceneEditor::SceneEditor(QWidget* parent, const char* name)
  : UI_SceneEditor(parent, name),
    m_deviceID  ( KNoID ),
    m_menu      (  NULL ),
    m_tempScene (  NULL )
{
  m_sceneList->setHScrollBarMode(QScrollView::AlwaysOff);
  initMenu();
}

SceneEditor::~SceneEditor()
{
  if (m_tempScene) delete m_tempScene;
}

void SceneEditor::setDevice(t_device_id id)
{
  m_deviceID = id;

  // The scene that contains all the edited values
  if (m_tempScene) delete m_tempScene;
  m_tempScene = new Scene();
  m_tempScene->setDevice(id);

  fillFunctions();
}

void SceneEditor::initMenu()
{
  m_tools->setPixmap(QPixmap(PIXMAPS + QString("/scene.png")));

  if (m_menu) delete m_menu;
  m_menu = new QPopupMenu();
  m_menu->insertItem(QPixmap(PIXMAPS + QString("/key.xpm")),
		     "Activate Selected", this, SLOT(slotActivate()),
		     0, KMenuActivate);
  m_menu->insertSeparator();
  m_menu->insertItem(QPixmap(PIXMAPS + QString("/filenew.xpm")),
		     "Create New...", this, SLOT(slotNew()), 
		     0, KMenuNew);
  m_menu->insertItem(QPixmap(PIXMAPS + QString("/filesave.xpm")),
		     "Overwrite", this, SLOT(slotStore()), 
		     0, KMenuStore);
  m_menu->insertItem(QPixmap(PIXMAPS + QString("/remove.xpm")),
		     "Remove Selected...", this, SLOT(slotRemove()), 
		     0, KMenuRemove);
  m_menu->insertItem(QPixmap(PIXMAPS + QString("/rename.xpm")),
		     "Rename Selected...", this, SLOT(slotRename()), 
		     0, KMenuRename);

  m_tools->setPopup(m_menu);
}

void SceneEditor::slotChannelChanged(t_channel channel, t_value value,
				     Scene::ValueType status)
{
  assert(m_tempScene);
  m_tempScene->set(channel, value, status);

  setStatusText(KStatusModified, KStatusColorModified);  
}

void SceneEditor::slotActivate()
{
  Scene* s = currentScene();

  if (s != NULL)
    {
      setScene(s);
      m_tempScene->copyFrom(s, s->device());
    }

  setStatusText(KStatusUnchanged, KStatusColorUnchanged);
}


void SceneEditor::setScene(Scene* scene)
{
  assert(scene);

  emit sceneActivated(scene->values(), scene->channels());
}

void SceneEditor::slotSceneListContextMenu(QListBoxItem* item,
					   const QPoint &point)
{
  m_menu->exec(point);
}

void SceneEditor::slotRemove()
{
  Scene* s = currentScene();

  if (s == NULL)
    {
      return;
    }

  if (QMessageBox::warning(this, "Scene Editor", "Remove selected scene?",
			   QMessageBox::Yes, QMessageBox::No) 
      == QMessageBox::Yes)
    {
      _app->doc()->deleteFunction(s->id());
      fillFunctions();
    }
}

void SceneEditor::slotRename()
{
  bool ok = false;
  Scene* s = currentScene();

  if (s == NULL)
    {
      return;
    }

  QString text = QInputDialog::getText("Scene editor - Rename Scene",
				       "Enter scene name",
				       QLineEdit::Normal,
				       s->name(), &ok, this);
  if (ok && !text.isEmpty())
    {
      s->setName(text);
      fillFunctions();

      selectFunction(s->id());
    }
}

void SceneEditor::slotNew()
{
  bool ok = false;
  QString text = QInputDialog::getText(tr("Scene editor - New Scene"),
				       tr("Enter scene name"),
				       QLineEdit::Normal,
				       QString::null, &ok, this);
  
  if (ok && !text.isEmpty())
    {
      Scene* sc = static_cast<Scene*>
	(_app->doc()->newFunction(Function::Scene));

      sc->copyFrom(m_tempScene, m_tempScene->device());
      sc->setName(text);
      
      ListBoxIDItem* item = new ListBoxIDItem();
      item->setText(sc->name());
      item->setRtti(sc->id());
      m_sceneList->insertItem(item);

      m_sceneList->sort();
      selectFunction(sc->id());
      m_sceneList->ensureCurrentVisible();
      
      setStatusText(KStatusStored, KStatusColorStored);
    }
}

void SceneEditor::slotStore()
{
  Scene* sc = currentScene();
  if (sc == NULL)
    {
      return;
    }

  // Save name & bus because copyFrom overwrites them
  QString name = sc->name();
  t_bus_id bus = sc->busID();

  sc->copyFrom(m_tempScene, m_tempScene->device());

  // Set these again
  sc->setName(name);
  sc->setBus(bus);

  setStatusText(KStatusStored, KStatusColorStored);
}

Scene* SceneEditor::currentScene()
{
  t_function_id fid = 0;

  if (m_sceneList->selectedItem() == NULL)
    {
      return NULL;
    }

  fid = static_cast<ListBoxIDItem*> (m_sceneList->selectedItem())->rtti();

  return static_cast<Scene*> (_app->doc()->function(fid));
}

void SceneEditor::fillFunctions()
{
  m_sceneList->clear();

  for (t_function_id id = 0; id < KFunctionArraySize; id++)
    {
      Function* f = _app->doc()->function(id);
      if (!f)
	continue;

      if (f->type() == Function::Scene && f->device() == m_deviceID)
	{
	  ListBoxIDItem* item = new ListBoxIDItem();
	  item->setText(f->name());
	  item->setRtti(f->id());
	  m_sceneList->insertItem(item);
	}
    }

  m_sceneList->sort();

  setStatusText(KStatusUnchanged, KStatusColorUnchanged);
}

void SceneEditor::selectFunction(t_function_id fid)
{
  for (unsigned int i = 0; i < m_sceneList->count(); i++)
    {
      ListBoxIDItem* item = static_cast<ListBoxIDItem*> (m_sceneList->item(i));
      if (static_cast<t_function_id> (item->rtti()) == fid)
	{
	  m_sceneList->setSelected(item, true);
	  m_sceneList->ensureCurrentVisible();
	  break;
	}
    }
}


void SceneEditor::setStatusText(QString text, QColor color)
{
  m_statusLabel->setPaletteForegroundColor(color);
  m_statusLabel->setText(text);
}
