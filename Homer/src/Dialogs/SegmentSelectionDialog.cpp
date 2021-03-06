/*****************************************************************************
 *
 * Copyright (C) 2010 Thomas Volkert <thomas@homer-conferencing.com>
 *
 * This software is free software.
 * Your are allowed to redistribute it and/or modify it under the terms of
 * the GNU General Public License version 2 as published by the Free Software
 * Foundation.
 *
 * This source is published in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License version 2 for more details.
 *
 * You should have received a copy of the GNU General Public License version 2
 * along with this program. Otherwise, you can write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 * Alternatively, you find an online version of the license text under
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 *****************************************************************************/

/*
 * Purpose: Implementation of a dialog for selecting the screen segment which is to be captured
 * Since:   2010-12-19
 */

#include <Dialogs/SegmentSelectionDialog.h>

#include <QDesktopWidget>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QDialog>
#include <QMenu>
#ifdef APPLE
#include <ApplicationServices/ApplicationServices.h>
#endif

namespace Homer { namespace Gui {

///////////////////////////////////////////////////////////////////////////////

// auto apply the settings?
#define SEGMENT_SELECTION_AUTO_APPLY

///////////////////////////////////////////////////////////////////////////////

SegmentSelectionDialog::SegmentSelectionDialog(QWidget* pParent, MediaSourceDesktop *pMediaSourceDesktop):
    QDialog(pParent, Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint)
{
    mMediaSourceDesktop = pMediaSourceDesktop;
}

SegmentSelectionDialog::~SegmentSelectionDialog()
{
}

void SegmentSelectionDialog::Init()
{
    initializeGUI();
}

///////////////////////////////////////////////////////////////////////////////

void SegmentSelectionDialog::initializeGUI()
{
    LOG(LOG_VERBOSE, "Found current segment resolution of %d*%d, starting at %d*%d", mMediaSourceDesktop->mSourceResX, mMediaSourceDesktop->mSourceResY, mMediaSourceDesktop->mGrabOffsetX, mMediaSourceDesktop->mGrabOffsetY);

    setupUi(this);

    connect(mTbDefaults, SIGNAL(clicked(bool)), this, SLOT(ResetToDefaults()));
    connect(mTbDesktopAuto, SIGNAL(clicked(bool)), this, SLOT(ResetToDesktopAuto(bool)));
    connect(mTbScreenAuto, SIGNAL(clicked(bool)), this, SLOT(ResetToScreenAuto(bool)));

    mTbScreenAuto->setChecked(mMediaSourceDesktop->GetAutoScreen());
    mTbDesktopAuto->setChecked(mMediaSourceDesktop->GetAutoDesktop());
    mCbMouse->setChecked(mMediaSourceDesktop->GetMouseVisualization());

    ConfigureDesktopCapturing(mMediaSourceDesktop->mGrabOffsetX, mMediaSourceDesktop->mGrabOffsetY, mMediaSourceDesktop->mSourceResX, mMediaSourceDesktop->mSourceResY, true);
}

int SegmentSelectionDialog::exec()
{
	int tResult;

	tResult = QDialog::exec();

	if (tResult == QDialog::Accepted)
	{
        mMediaSourceDesktop->SetAutoScreen(mTbScreenAuto->isChecked());
		mMediaSourceDesktop->SetAutoDesktop(mTbDesktopAuto->isChecked());
		mMediaSourceDesktop->SetMouseVisualization(mCbMouse->isChecked());

		mMediaSourceDesktop->mGrabOffsetX = mOffsetX;
		mMediaSourceDesktop->mGrabOffsetY = mOffsetY;
		mMediaSourceDesktop->mSourceResX = mWidth;
		mMediaSourceDesktop->mSourceResY = mHeight;
	}

	return tResult;
}

void SegmentSelectionDialog::ConfigureDesktopCapturing(int pOffsetX, int pOffsetY, int pWidth, int pHeight, bool pForceUpdate)
{
    //LOG(LOG_ERROR, "Configuring desktop capturing to offset (%d,%d) and dimension %d*%d", pOffsetX, pOffsetY, pWidth, pHeight);

    if ((pWidth >= 0) && (pHeight >= 0) && ((mWidth != pHeight) || (mHeight != pHeight) || (pForceUpdate)))
    {
        mWidth = pWidth;
        mHeight = pHeight;

        #ifdef SEGMENT_SELECTION_AUTO_APPLY
            mMediaSourceDesktop->mSourceResX = pWidth;
            mMediaSourceDesktop->mSourceResY = pHeight;
        #endif

        mLbResX->setText(QString("%1").arg(pWidth));
        mLbResY->setText(QString("%1").arg(pHeight));

        resize(pWidth, pHeight);
    }

    if ((pOffsetX >= 0) && (pOffsetY >= 0) && ((mOffsetX != pOffsetX) || (mOffsetY != pOffsetY) || (pForceUpdate)))
    {
        mOffsetX = pOffsetX;
        mOffsetY = pOffsetY;

        #ifdef SEGMENT_SELECTION_AUTO_APPLY
            mMediaSourceDesktop->mGrabOffsetX = pOffsetX;
            mMediaSourceDesktop->mGrabOffsetY = pOffsetY;
        #endif

        mLbOffsetX->setText(QString("%1").arg(pOffsetX));
        mLbOffsetY->setText(QString("%1").arg(pOffsetY));

        move(pOffsetX, pOffsetY);
    }
}

void SegmentSelectionDialog::ResetToScreen()
{
    LOG(LOG_VERBOSE, "Resetting to screen size");

    int tResX = 0;
    int tResY = 0;
    #ifdef APPLE
        tResX = CGDisplayPixelsWide(CGMainDisplayID());
        tResY = CGDisplayPixelsHigh(CGMainDisplayID());
    #else
        QDesktopWidget *tDesktop = QApplication::desktop();
        tResX = tDesktop->screenGeometry(tDesktop->primaryScreen()).width();
        tResY = tDesktop->screenGeometry(tDesktop->primaryScreen()).height();
    #endif

    ConfigureDesktopCapturing(0, 0, tResX, tResY);
}
void SegmentSelectionDialog::ResetToDesktop()
{
    LOG(LOG_VERBOSE, "Resetting to desktop size");

    int tResX = 0;
    int tResY = 0;
    QDesktopWidget *tDesktop = QApplication::desktop();
    tResX = tDesktop->availableGeometry(tDesktop->primaryScreen()).width();
    tResY = tDesktop->availableGeometry(tDesktop->primaryScreen()).height();

    ConfigureDesktopCapturing(0, 0, tResX, tResY);
}

void SegmentSelectionDialog::ResetToScreenAuto(bool pActive)
{
    if (pActive)
        ResetToScreen();
    mTbDesktopAuto->setChecked(false);
}

void SegmentSelectionDialog::ResetToDesktopAuto(bool pActive)
{
	if (pActive)
		ResetToDesktop();
	mTbScreenAuto->setChecked(false);
}

void SegmentSelectionDialog::ResetToDefaults()
{
    LOG(LOG_VERBOSE, "Resetting to defaults");
    mTbDesktopAuto->setChecked(false);
    mTbScreenAuto->setChecked(false);
    ConfigureDesktopCapturing(0, 0, DESKTOP_SEGMENT_MIN_WIDTH, DESKTOP_SEGMENT_MIN_HEIGHT);
}

void SegmentSelectionDialog::ClickedButton(QAbstractButton *pButton)
{
    if (mBb->standardButton(pButton) == QDialogButtonBox::Reset)
    {
    	ResetToDefaults();
    }
}

void SegmentSelectionDialog::contextMenuEvent(QContextMenuEvent *event)
{
    QAction *tAction;

    QMenu tMenu(this);

    tAction = tMenu.addAction(Homer::Gui::SegmentSelectionDialog::tr("Default settings"));
    QIcon tIcon;
    tIcon.addPixmap(QPixmap(":/images/22_22/Reload.png"), QIcon::Normal, QIcon::Off);
    tAction->setIcon(tIcon);

    QAction* tPopupRes = tMenu.exec(QCursor::pos());
    if (tPopupRes != NULL)
    {
        if (tPopupRes->text().compare(Homer::Gui::SegmentSelectionDialog::tr("Default settings")) == 0)
        {
            ResetToDefaults();
            return;
        }
    }
}

void SegmentSelectionDialog::mousePressEvent(QMouseEvent *pEvent)
{
    //LOG(LOG_WARN, "mousePressEvent");

    if (pEvent->button() == Qt::LeftButton)
    {
        mDragMousePosition = pEvent->globalPos();
        mDragWindowPosition = pos();
        pEvent->accept();
    }
}

void SegmentSelectionDialog::mouseMoveEvent(QMouseEvent *pEvent)
{
    //LOG(LOG_WARN, "mouseMoveEvent");

    if (pEvent->buttons() & Qt::LeftButton)
    {
        QPoint tNewPos = mDragWindowPosition + pEvent->globalPos() - mDragMousePosition;
        if (tNewPos.x() < 0)
            tNewPos.setX(0);
        if (tNewPos.y() < 0)
            tNewPos.setY(0);

        ConfigureDesktopCapturing(tNewPos.x(), tNewPos.y(), -1, -1);

        pEvent->accept();
    }
}

void SegmentSelectionDialog::resizeEvent(QResizeEvent *pEvent)
{
    int tNewWidth = pEvent->size().width();
    if (tNewWidth < DESKTOP_SEGMENT_MIN_WIDTH)
        tNewWidth = DESKTOP_SEGMENT_MIN_WIDTH;

    int tNewHeight = pEvent->size().height();
    if (tNewHeight < DESKTOP_SEGMENT_MIN_HEIGHT)
        tNewHeight = DESKTOP_SEGMENT_MIN_HEIGHT;

    ConfigureDesktopCapturing( -1, -1, tNewWidth, tNewHeight);

    QDialog::resizeEvent(pEvent);
}

///////////////////////////////////////////////////////////////////////////////

}} //namespace
