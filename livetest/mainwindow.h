#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <obs.hpp>
#include "window-basic-main-outputs.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define PREVIEW_EDGE_SIZE 10

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	OBSScene GetCurrentScene();
	obs_service_t *GetService();

	void resetOutputs();

private slots:
	void sourceMonitorClick();
	void sourceVideoClick();
	void sourceImageClick();
	void sourceCameraClick();
	void sourceTextClick();
	void sourceAudioInput();
	void sourceAudioOutput();
	void startPushClick();
	void stopPushClick();

	void StreamDelayStarting(int sec);
	void StreamDelayStopping(int sec);
	void StreamingStart();
	void StreamStopping();
	void StreamingStop(int errorcode, QString last_error);

private:
	static void RenderMain(void *data, uint32_t cx, uint32_t cy);
	void InitPrimitives();
	void InitService();
	void ResizePreview(uint32_t cx, uint32_t cy);
public:
	int previewX = 0, previewY = 0;
	int previewCX = 640, previewCY = 360;
	float previewScale = 1.0f;

	gs_vertbuffer_t *box = nullptr;
	gs_vertbuffer_t *boxLeft = nullptr;
	gs_vertbuffer_t *boxTop = nullptr;
	gs_vertbuffer_t *boxRight = nullptr;
	gs_vertbuffer_t *boxBottom = nullptr;
	gs_vertbuffer_t *circle = nullptr;

private:
	OBSScene currentScene;
	OBSService service;
	std::unique_ptr<BasicOutputHandler> outputHandler;
	Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
