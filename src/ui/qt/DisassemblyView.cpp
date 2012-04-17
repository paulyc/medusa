#include "DisassemblyView.hpp"

DisassemblyView::DisassemblyView(QWidget * parent)
  : QAbstractScrollArea(parent)
  , _db(nullptr)
  , _xOffset(0),        _yOffset(10)
  , _xData(0),          _yData(0)
  , _wChar(0),          _hChar(0)
  , _xCursor(0),        _yCursor(0)
  , _firstSelection(0), _lastSelection(0)
  , _addrLen(0)
  , _lineNo(0x10000),   _lineLen(0x100)
  , _cursorTimer(),     _cursorBlink(false)
{
  setFont(QFont("consolas", 10));

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  connect(&_cursorTimer, SIGNAL(timeout()), this, SLOT(updateCursor()));
  _cursorTimer.setInterval(400);
}

DisassemblyView::~DisassemblyView(void)
{
}

void DisassemblyView::setDatabase(medusa::Database const * db)
{
  _db = db;
}

bool DisassemblyView::goTo(medusa::Database::View::LineInformation const & lineInfo)
{
  int line;
  if (!_db->GetView().ConvertLineInformationToLine(lineInfo, line)) return false;
  verticalScrollBar()->setValue(line);
  return true;
}

void DisassemblyView::setFont(QFont const & font)
{
  QAbstractScrollArea::setFont(font);
  const QFontMetrics metrics(font);

  _wChar = metrics.width('X');
  _hChar = metrics.height();

  updateScrollbars();
  listingUpdated();
}

void DisassemblyView::listingUpdated(void)
{
  viewport()->update();
}

void DisassemblyView::updateCursor(void)
{
  _cursorBlink = _cursorBlink ? false : true;
  emit listingUpdated();
}

void DisassemblyView::paintEvent(QPaintEvent * evt)
{
  QPainter p(viewport());

  // Draw background
  QColor addrColor = QColor(Settings::instance().value(MEDUSA_COLOR_ADDRESS_BACKGROUND, MEDUSA_COLOR_ADDRESS_BACKGROUND_DEFAULT).toString());
  QColor codeColor = QColor(Settings::instance().value(MEDUSA_COLOR_VIEW_BACKGROUND, MEDUSA_COLOR_VIEW_BACKGROUND_DEFAULT).toString());

  viewport()->rect();
  QRect addrRect = viewport()->rect();
  QRect codeRect = viewport()->rect();

  addrRect.setWidth((_addrLen - horizontalScrollBar()->value()) * _wChar);
  codeRect.setX((_addrLen - horizontalScrollBar()->value()) * _wChar);

  p.fillRect(addrRect, addrColor);
  p.fillRect(codeRect, codeColor);

  if (_db == nullptr) return;

  typedef medusa::Database::View::LineInformation LineInformation;

  QColor color;

  p.setRenderHints(QPainter::TextAntialiasing);

  LineInformation lineInfo;

  int endLine = viewport()->rect().height() / _hChar + 1;
  int curLine = verticalScrollBar()->value();

  for (int line = 0; line < endLine && _db->GetView().GetLineInformation(line + curLine, lineInfo); ++line)
    _addrLen = qMax(static_cast<int>(lineInfo.GetAddress().ToString().length() + 1), _addrLen);

  int offLine = (_addrLen + 1) * _wChar;
  int begLine = -(horizontalScrollBar()->value() * _wChar);

  // Draw address lines
  for (int line = 0; line < endLine && _db->GetView().GetLineInformation(line + curLine, lineInfo); ++line)
  {
    p.drawText(begLine, _yOffset + line * _hChar, QString::fromStdString(lineInfo.GetAddress().ToString()));
  }

  // Draw assembly code lines
  for (int line = 0; line < endLine && _db->GetView().GetLineInformation(line + curLine, lineInfo); ++line)
  {
    QString lineStr = "Invalid line !";
    switch (lineInfo.GetType())
    {
    case LineInformation::CellLineType:
      {
        medusa::Cell const * curCell = _db->RetrieveCell(lineInfo.GetAddress());
        if (curCell == nullptr) break;

        medusa::u16 offset = 0;

        medusa::Cell::Mark::List marks = curCell->GetMarks();
        if (marks.empty())
        {
          color = Qt::black;
          lineStr = QString::fromStdString(curCell->ToString());
          break;
        }

        std::for_each(std::begin(marks), std::end(marks), [&](medusa::Cell::Mark mark)
        {
          QColor cellClr(Qt::black);
          QString cellStr = QString::fromStdString(curCell->ToString().substr(offset, mark.GetLength()));

          switch (mark.GetType())
          {
          case medusa::Cell::Mark::MnemonicType:  cellClr = QColor(Settings::instance().value(MEDUSA_COLOR_INSTRUCTION_MNEMONIC, MEDUSA_COLOR_INSTRUCTION_MNEMONIC_DEFAULT).toString()); break;
          case medusa::Cell::Mark::KeywordType:   cellClr = QColor(Settings::instance().value(MEDUSA_COLOR_INSTRUCTION_KEYWORD, MEDUSA_COLOR_INSTRUCTION_KEYWORD_DEFAULT).toString()); break;
          case medusa::Cell::Mark::ImmediateType: cellClr = QColor(Settings::instance().value(MEDUSA_COLOR_INSTRUCTION_IMMEDIATE, MEDUSA_COLOR_INSTRUCTION_IMMEDIATE_DEFAULT).toString()); break;
          case medusa::Cell::Mark::OperatorType:  cellClr = QColor(Settings::instance().value(MEDUSA_COLOR_INSTRUCTION_OPERATOR, MEDUSA_COLOR_INSTRUCTION_OPERATOR_DEFAULT).toString()); break;
          case medusa::Cell::Mark::RegisterType:  cellClr = QColor(Settings::instance().value(MEDUSA_COLOR_INSTRUCTION_REGISTER, MEDUSA_COLOR_INSTRUCTION_REGISTER_DEFAULT).toString()); break;
          case medusa::Cell::Mark::LabelType:     cellClr = QColor(Settings::instance().value(MEDUSA_COLOR_INSTRUCTION_LABEL, MEDUSA_COLOR_INSTRUCTION_LABEL_DEFAULT).toString()); break;
          case medusa::Cell::Mark::StringType:    cellClr = QColor(Settings::instance().value(MEDUSA_COLOR_INSTRUCTION_STRING, MEDUSA_COLOR_INSTRUCTION_STRING_DEFAULT).toString()); break;
          default: break;
          };

          p.setPen(cellClr);
          p.drawText(begLine + offset * _wChar + offLine, _yOffset + line * _hChar, cellStr);
          offset += mark.GetLength();
        });

        lineStr = "";
        if (!curCell->GetComment().empty())
        {
          p.setPen(QColor(Settings::instance().value(MEDUSA_COLOR_INSTRUCTION_COMMENT, MEDUSA_COLOR_INSTRUCTION_COMMENT_DEFAULT).toString()));
          p.drawText(begLine + offset * _wChar + offLine, _yOffset + line * _hChar, QString(" ; ") + QString::fromStdString(curCell->GetComment()));
        }
        break;
      }

    case LineInformation::MultiCellLineType:
      {
        medusa::MultiCell const *curMultiCell = _db->RetrieveMultiCell(lineInfo.GetAddress());
        color = QColor(Settings::instance().value(MEDUSA_COLOR_INSTRUCTION_COMMENT, MEDUSA_COLOR_INSTRUCTION_COMMENT_DEFAULT).toString());

        if (curMultiCell != nullptr)
          lineStr = QString::fromStdString(curMultiCell->ToString());
        break;
      }

    case LineInformation::LabelLineType:
      {
        medusa::Label curLabel = _db->GetLabelFromAddress(lineInfo.GetAddress());
        color = QColor(Settings::instance().value(MEDUSA_COLOR_INSTRUCTION_LABEL, MEDUSA_COLOR_INSTRUCTION_LABEL_DEFAULT).toString());

        if (curLabel.GetType() != medusa::Label::LabelUnknown)
          lineStr = QString::fromStdString(curLabel.GetLabel()) + QString(":");
        break;
      }

    case LineInformation::XrefLineType:
      {
        medusa::Address::List RefAddrList;
        _db->GetXRefs().From(lineInfo.GetAddress(), RefAddrList);
        lineStr = QString::fromUtf8(";:xref");

        std::for_each(std::begin(RefAddrList), std::end(RefAddrList), [&](medusa::Address const& rRefAddr)
        {
          lineStr += QString(" ") + (rRefAddr < lineInfo.GetAddress() ? QString::fromUtf8("\xe2\x86\x91") : QString::fromUtf8("\xe2\x86\x93")) + QString::fromStdString(rRefAddr.ToString());
        });
        color = QColor(Settings::instance().value(MEDUSA_COLOR_INSTRUCTION_KEYWORD, MEDUSA_COLOR_INSTRUCTION_KEYWORD_DEFAULT).toString());
        break;
      }

    case LineInformation::MemoryAreaLineType:
      {
        medusa::MemoryArea const* memArea = _db->GetMemoryArea(lineInfo.GetAddress());
        if (memArea == nullptr) break;
        lineStr = QString::fromStdString(memArea->ToString());
        color = Qt::magenta;
        break;
      }

    case LineInformation::EmptyLineType: continue;
    default: break;
    }

    if (lineStr.isEmpty()) continue;

    p.setPen(color);
    p.drawText(begLine + offLine, _yOffset + line * _hChar, lineStr);
  }

  // Draw cursor
  if (_cursorBlink)
  {
    QColor cursorColor = ~codeColor.value();
    QRect cursorRect((_xCursor - horizontalScrollBar()->value()) * _wChar, (_yCursor - verticalScrollBar()->value()) * _hChar, 2, _hChar);
    p.fillRect(cursorRect, cursorColor);
    qDebug() << "x: " << _xCursor * _wChar << ", y: " << _yCursor * _hChar;
  }

  updateScrollbars();
}

void DisassemblyView::mouseMoveEvent(QMouseEvent * evt)
{
  medusa::Address addr;

  if (!convertMouseToAddress(evt, addr)) return;
  qDebug() << QString::fromStdString(addr.ToString());
}

void DisassemblyView::mousePressEvent(QMouseEvent * evt)
{
  _cursorBlink = true;
  int xCursor = evt->x() / _wChar + horizontalScrollBar()->value();
  int yCursor = evt->y() / _hChar + verticalScrollBar()->value();

  qDebug() << "hsb: " << horizontalScrollBar()->value() << ", vsb: " << verticalScrollBar()->value();
  qDebug() << "xcs: " << xCursor << "ycs: " << yCursor;

  if (xCursor > _addrLen)
  {
    _xCursor = xCursor;
    _yCursor = yCursor;
  }
  _cursorTimer.start();
}

void DisassemblyView::mouseDoubleClickEvent(QMouseEvent * evt)
{
  medusa::Address srcAddr, dstAddr;

  if (!convertMouseToAddress(evt, srcAddr)) return;

  medusa::Cell const* cell = _db->RetrieveCell(srcAddr);
  if (cell == nullptr) return;

  typedef medusa::Database::View::LineInformation LineInformation;

  auto memArea = _db->GetMemoryArea(srcAddr);

  for (medusa::u8 op = 0; op < 4; ++op)
  {
    if ( memArea != nullptr
      && cell->GetType() == medusa::Cell::InstructionType
      && static_cast<medusa::Instruction const*>(cell)->GetOperandReference(*_db, op, srcAddr, dstAddr))
      if (goTo(LineInformation(LineInformation::CellLineType, dstAddr)))
        return;
  }
}

void DisassemblyView::updateScrollbars(void)
{
  if (_db == nullptr) return;

  int numberOfLine = static_cast<int>(_db->GetView().GetNumberOfLine());
  if (numberOfLine == 0) return;

  int max = numberOfLine - (viewport()->rect().height() / _hChar);
  if (max < 0) max = 0;

  verticalScrollBar()->setMaximum(max);
  horizontalScrollBar()->setMaximum(_lineLen);
}

bool DisassemblyView::convertMouseToAddress(QMouseEvent * evt, medusa::Address & addr)
{
  int line = evt->pos().y() / _hChar + verticalScrollBar()->value();
  medusa::Database::View::LineInformation lineInfo;

  if (!_db->GetView().GetLineInformation(line, lineInfo)) return false;

  addr = lineInfo.GetAddress();
  return true;
}