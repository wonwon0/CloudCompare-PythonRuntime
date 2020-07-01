#include "QPythonREPL.h"
#include "PythonHighlighter.h"

#include <ui_PythonREPLQt.h>
#include <ccLog.h>

#undef slots

#include "PythonStdErrOutRedirect.h"

namespace py = pybind11;

const static QString replArrows = ">>> ";
const static QString continuationDots = "... ";

bool ui::KeyPressEater::eventFilter(QObject *obj, QEvent *event)
{
	if ( event->type() == QEvent::KeyPress )
	{
		auto *keyEvent = static_cast<QKeyEvent *>(event);
		qDebug("Ate key press %d", keyEvent->key());
		if ( keyEvent->modifiers() & Qt::ShiftModifier &&
		     (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return))
		{
			m_repl->codeEdit()->appendPlainText(continuationDots);
			return true;
		}
		QTextCursor cursor = m_repl->codeEdit()->textCursor();

		switch (keyEvent->key())
		{
			case Qt::Key_Enter:
				Q_FALLTHROUGH();
			case Qt::Key_Return:
			{
				if ( m_repl->codeEdit()->document()->characterCount() < replArrows.size())
				{
					return true;
				}

				// Try to be smart, create a new line if the python code will need one
				if ( m_repl->codeEdit()->document()->characterAt(
						m_repl->codeEdit()->document()->characterCount() - 2) ==
				     ":" )
				{
					m_repl->codeEdit()->appendPlainText(continuationDots);
					return true;
				}

				const QString pythonCode = m_repl->codeEdit()->toPlainText();
				m_repl->executeCode(pythonCode);
				m_repl->m_history.add(std::move(pythonCode));
				return true;
			}
			case Qt::Key_Backspace:
				Q_FALLTHROUGH();
			case Qt::Key_Delete:
				if ( cursor.columnNumber() > replArrows.size())
				{
					return QObject::eventFilter(obj, event);
				} else
				{
					return true;
				}
			case Qt::Key_Left:
				if ( cursor.columnNumber() > replArrows.size())
				{
					return QObject::eventFilter(obj, event);
				}
				return true;
			case Qt::Key_Down:
				if ( cursor.blockNumber() < m_repl->codeEdit()->blockCount() - 1 )
				{
					return QObject::eventFilter(obj, event);
				} else if ( !m_repl->m_history.empty())
				{
					m_repl->codeEdit()->clear();
					m_repl->codeEdit()->insertPlainText(m_repl->m_history.newer());
				}
				return true;
			case Qt::Key_Up:
				if ( cursor.blockNumber() > 0 )
				{
					return QObject::eventFilter(obj, event);
				} else if ( !m_repl->m_history.empty())
				{
					m_repl->codeEdit()->clear();
					m_repl->codeEdit()->insertPlainText(m_repl->m_history.older());
					return true;
				}
			case Qt::Key_Home:
				m_repl->codeEdit()->moveCursor(QTextCursor::StartOfLine);
				m_repl->codeEdit()->moveCursor(QTextCursor::NextWord);
				return true;
			default:
			{
				int posDiff = replArrows.size() - cursor.columnNumber();
				if ( posDiff > 0 )
				{
					cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, posDiff);
					m_repl->codeEdit()->setTextCursor(cursor);
				}
				return QObject::eventFilter(obj, event);
			}
		}

	} else
	{
		return QObject::eventFilter(obj, event);
	}
}

ui::KeyPressEater::KeyPressEater(QPythonREPL *repl, QObject *parent) : QObject(parent), m_repl(repl)
{

}

ui::QPythonREPL::QPythonREPL(QWidget *parent) :
		QWidget(parent),
		m_ui(new Ui_PythonREPLQt)
{

	m_ui->setupUi(this);
	auto keyPressEater = new KeyPressEater(this);
	codeEdit()->installEventFilter(keyPressEater);
	codeEdit()->resize(codeEdit()->width(), 20);

	auto highlighter = new PythonHighlighter(codeEdit()->document());
	codeEdit()->setTabStopWidth(codeEdit()->fontMetrics().width(' ') * 8);


	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);

	codeEdit()->setFont(font);

	m_buf.reserve(255);
	m_output = py::module::import("ccinternals")
			.attr("ConsoleREPL")(outputDisplay());

	executeCode(replArrows + "import pycc");
	executeCode(replArrows + "cc = pycc.GetInstance()");
}

ui::QPythonREPL::~QPythonREPL()
{
	delete m_ui;
}

QPlainTextEdit *ui::QPythonREPL::codeEdit()
{
	return m_ui->codeEdit;
}

QListWidget *ui::QPythonREPL::outputDisplay()
{
	return m_ui->outputDisplay;
}

void ui::QPythonREPL::executeCode(const QString &pythonCode)
{
	/// Our raw input contains the ">>> " and "... " and is a QString
	/// convert it to std::string while filtering out non needed chars
	m_buf.clear();
	QString::const_iterator iter = pythonCode.begin() + replArrows.size();
	while (iter < pythonCode.end())
	{
		// FIXME this will return 0 on non latin chars
		//  we should do better
		char c = iter->toLatin1();
		if ( c == 0 )
		{
			outputDisplay()->addItem("Input contains non Latin1 chars");
			return;
		}
		m_buf += c;
		if ( *iter == '\n' )
		{
			iter += continuationDots.size();
		}
		iter++;
	}
	ccLog::Print(m_buf.c_str());
	outputDisplay()->addItem(pythonCode);
	codeEdit()->clear();
	codeEdit()->insertPlainText(replArrows);
	try
	{
		PyStdErrOutStreamRedirect redir{m_output, m_output};
		py::exec(m_buf.c_str(), py::globals(), m_locals);
//				 This part is only useful if we use py::eval
//				 however py::eval does not work on assignent expr (eg: my_var = 1)
//				const auto result = py::eval...
//				if (!result.is_none()) {
//					const std::string repr = result.attr("__repr__")().cast<std::string>();
//					outputDisplay()->addItem(QString::fromStdString(repr));
//				}
	} catch (const std::exception &e)
	{
		outputDisplay()->addItem(e.what());
	}
}

ui::History::History()
{}

void ui::History::add(const QString &&cmd)
{
	m_commands.push_back(cmd);
	m_current = m_commands.rbegin();
}

const QString &ui::History::older()
{
	if ( m_current == m_commands.rend())
	{
		m_current = m_commands.rbegin();
	}
	const QString &current = *m_current;
	m_current++;
	return current;
}

const QString &ui::History::newer()
{
	if ( m_current == m_commands.rend())
	{
		return replArrows;
	}

	const QString &current = *m_current;
	if ( m_current == m_commands.rbegin())
	{
		m_current = m_commands.rend();
	} else
	{

		m_current--;
	}
	return current;
}

bool ui::History::empty() const
{
	return m_commands.empty();
}

size_t ui::History::size() const
{
	return m_commands.size();
}
