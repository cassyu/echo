#pragma once

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QWidget>
#include <QPainter>
#include <functional>
#include "engine/core/util/StringUtil.h"

namespace QT_UI
{
	class QResEdit : public QWidget
	{
		Q_OBJECT

	public:
		using OpenFileDialogFunction = std::function < Echo::String(QWidget*, const char*, const char*, const char*) >;

	public:
		QResEdit(class QPropertyModel* model, QString propertyName, const char* exts, const char* files, QWidget* parent = 0);

		// 设置路径
		void SetPath( QString text) { m_lineEdit->setText( text);  }

		// 获取路径
		QString GetPath() { return m_lineEdit->text(); }

		// set open operation
		static void setOpenFileDialogFunction(const OpenFileDialogFunction& func) { m_openFileFunction = func; }

		// MVC渲染
		static void ItemDelegatePaint(QPainter *painter, const QRect& rect, const Echo::String& val);

	protected:
		// redefine paintEvent
		void paintEvent(QPaintEvent* event);

		// is texture res
		bool isTextureRes();

		// correct size
		void adjustHeightSize();

	private slots:
		// 选择路径
		void OnSelectPath();

		// edit finished
		void onEditFinished();

	private:
		Echo::String	m_exts;
		QString			m_files;
		QHBoxLayout*	m_horizonLayout;
		QLineEdit*		m_lineEdit;
		QToolButton*	m_toolButton;
		static OpenFileDialogFunction m_openFileFunction;
		QPropertyModel* m_propertyModel;
		QString			m_propertyName;
	};
}