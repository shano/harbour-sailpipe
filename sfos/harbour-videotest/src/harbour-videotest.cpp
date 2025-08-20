#include <QtQuick>

#include <sailfishapp.h>

#include "videomanagement.h"

int main(int argc, char *argv[])
{
  int result;

  QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
  QCoreApplication::setOrganizationName("uk.co.flypig");
  QCoreApplication::setApplicationName("harbour-videotest");
  QScopedPointer<QQuickView> view(SailfishApp::createView());
  QQmlContext* ctxt = view->rootContext();
  //QQmlEngine* engine = view->engine();
  QScopedPointer<VideoManagement> videoManagement(new VideoManagement(app.data()));

  qmlRegisterType<VideoManagement>("uk.co.flypig.videotest", 1, 0, "VideoManagement");

  ctxt->setContextProperty("videoManagement", videoManagement.data());

  view->setSource(SailfishApp::pathTo("qml/harbour-videotest.qml"));
  view->show();
  result = app->exec();

  return result;
}
