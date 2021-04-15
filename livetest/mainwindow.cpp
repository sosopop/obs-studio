#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <graphics/vec2.h>
#include <util/base.h>
#include <media-io/audio-resampler.h>
#include <obs.h>
#include <qdebug.h>

#include <intrin.h>
#include <util/profiler.hpp>
#ifdef WIN32
#include <windows.h>
#endif
#include "qt-display.hpp"
#include "display-helpers.hpp"

#define DL_OPENGL "libobs-opengl.dll"
#define DL_D3D11 "libobs-d3d11.dll"

MainWindow *mainWindow = nullptr;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	mainWindow = this;

	struct obs_video_info ovi;
	ovi.adapter = 0;
	ovi.base_width = 1920;
	ovi.base_height = 1080;
	ovi.fps_num = 30;
	ovi.fps_den = 1;
	ovi.graphics_module = DL_OPENGL;
	ovi.output_format = VIDEO_FORMAT_NV12;
	ovi.output_width = 1280;
	ovi.output_height = 720;
	ovi.colorspace = VIDEO_CS_709;
	ovi.range = VIDEO_RANGE_PARTIAL;

	if (obs_reset_video(&ovi) != 0)
		throw "Couldn't initialize video";

	struct obs_audio_info oai;
	oai.samples_per_sec = 48000;
	oai.speakers = SPEAKERS_STEREO;
	obs_reset_audio(&oai);

	currentScene = obs_scene_create("test scene");
	obs_scene_release(currentScene);
	obs_set_output_source(0, obs_scene_get_source(currentScene));

	InitPrimitives();
	resetOutputs();
	InitService();

	auto addDisplay = [this](OBSQTDisplay *window) {
		obs_display_add_draw_callback(window->GetDisplay(),
					      MainWindow::RenderMain, this);

		struct obs_video_info ovi;
		if (obs_get_video_info(&ovi))
			ResizePreview(ovi.base_width, ovi.base_height);
	};

	connect(ui->preview, &OBSQTDisplay::DisplayCreated, addDisplay);
	connect(ui->btnSourceMonitor, SIGNAL(clicked()), this,
		SLOT(sourceMonitorClick()));
	connect(ui->btnSourceVideo, SIGNAL(clicked()), this,
		SLOT(sourceVideoClick()));
	connect(ui->btnSourceImage, SIGNAL(clicked()), this,
		SLOT(sourceImageClick()));
	connect(ui->btnSourceCamera, SIGNAL(clicked()), this,
		SLOT(sourceCameraClick()));
	connect(ui->btnSourceText, SIGNAL(clicked()), this,
		SLOT(sourceTextClick()));
	connect(ui->btnSourceAudioInput, SIGNAL(clicked()), this,
		SLOT(sourceAudioInput()));
	connect(ui->btnSourceAudioOutput, SIGNAL(clicked()), this,
		SLOT(sourceAudioOutput()));
	connect(ui->btnStartPush, SIGNAL(clicked()), this,
		SLOT(startPushClick()));
	connect(ui->btnStopPush, SIGNAL(clicked()), this,
		SLOT(stopPushClick()));

	auto displayResize = [this]() {
		struct obs_video_info ovi;

		if (obs_get_video_info(&ovi))
			ResizePreview(ovi.base_width, ovi.base_height);
	};
	connect(ui->preview, &OBSQTDisplay::DisplayResized, displayResize);
}

MainWindow::~MainWindow()
{
	obs_display_remove_draw_callback(ui->preview->GetDisplay(),
					 MainWindow::RenderMain, this);

	obs_enter_graphics();
	gs_vertexbuffer_destroy(box);
	gs_vertexbuffer_destroy(boxLeft);
	gs_vertexbuffer_destroy(boxTop);
	gs_vertexbuffer_destroy(boxRight);
	gs_vertexbuffer_destroy(boxBottom);
	gs_vertexbuffer_destroy(circle);
	obs_leave_graphics();

	delete ui;
}

OBSScene MainWindow::GetCurrentScene()
{
	return currentScene;
}

obs_service_t *MainWindow::GetService()
{
	return service;
}

void MainWindow::resetOutputs()
{
	outputHandler.reset();
	outputHandler.reset(CreateSimpleOutputHandler(this));
}

void MainWindow::sourceMonitorClick()
{
	auto source = obs_source_create("monitor_capture",
					"monitor_capture test", NULL, nullptr);
	obs_enter_graphics();
	obs_scene_atomic_update(
		currentScene,
		[](void *_data, obs_scene_t *scene) {
			obs_source_t *source =
				reinterpret_cast<obs_source_t *>(_data);
			auto item = obs_scene_add(scene, source);
			obs_sceneitem_set_visible(item, true);
			struct vec2 scale;
			vec2_set(&scale, 0.2f, 0.2f);
			obs_sceneitem_set_scale(item, &scale);
		},
		source);
	obs_leave_graphics();
	obs_source_release(source);
}

void MainWindow::sourceVideoClick()
{
	auto source = obs_source_create("ffmpeg_source", "ffmpeg_source test",
					NULL, nullptr);
	obs_data_t *settings = obs_data_create();
	obs_data_set_string(
		settings, "local_file",
		"D:/temp/Avengers- Endgame but only Best Scenes (4K 60FPS).mp4");
	obs_data_set_bool(settings, "looping", true);
	obs_source_update(source, settings);
	obs_data_release(settings);
	obs_enter_graphics();
	obs_scene_atomic_update(
		currentScene,
		[](void *_data, obs_scene_t *scene) {
			obs_source_t *source =
				reinterpret_cast<obs_source_t *>(_data);
			auto item = obs_scene_add(scene, source);
			obs_sceneitem_set_visible(item, true);
			struct vec2 scale;
			vec2_set(&scale, 0.2f, 0.2f);
			obs_sceneitem_set_scale(item, &scale);
		},
		source);
	obs_leave_graphics();
	obs_source_release(source);
}

void MainWindow::sourceImageClick()
{
	obs_data_t *settings = obs_data_create();
	auto source = obs_source_create("image_source", "image_source test",
					NULL, nullptr);
	obs_data_set_string(
		settings, "file",
		"C:/Users/mengchao/Desktop/gnys-face1.78509bab.png");
	obs_source_update(source, settings);
	obs_data_release(settings);
	obs_enter_graphics();
	obs_scene_atomic_update(
		currentScene,
		[](void *_data, obs_scene_t *scene) {
			obs_source_t *source =
				reinterpret_cast<obs_source_t *>(_data);
			obs_scene_add(scene, source);
		},
		source);
	obs_leave_graphics();
	obs_source_release(source);
}

void MainWindow::sourceCameraClick()
{
	auto source = obs_source_create(
		"dshow_input", "dshow_input_source test", NULL, nullptr);
	auto properties = obs_source_properties(source);
	obs_property_t *property = obs_properties_first(properties);
	while (property) {
		const char *prop_name = obs_property_name(property);
		const char *desc = obs_property_description(property);
		UNUSED_PARAMETER(desc);
		enum obs_property_type type = obs_property_get_type(property);
		enum obs_combo_format format =
			obs_property_list_format(property);
		if (qstrcmp("video_device_id", prop_name) == 0 &&
		    OBS_PROPERTY_LIST == type &&
		    OBS_COMBO_FORMAT_STRING == format) {
			size_t count = obs_property_list_item_count(property);
			for (size_t i = 0; i < count; i++) {
				const char *prop_list_name =
					obs_property_list_item_name(property,
								    i);
				const char *value =
					obs_property_list_item_string(property,
								      i);
				qDebug() << prop_list_name << ":" << value;
				obs_data_t *settings = obs_data_create();
				obs_data_set_string(settings, "video_device_id",
						    value);
				obs_source_update(source, settings);
				obs_data_release(settings);
				break;
			}
		}
		obs_property_next(&property);
	}
	obs_enter_graphics();
	obs_scene_atomic_update(
		currentScene,
		[](void *_data, obs_scene_t *scene) {
			obs_source_t *source =
				reinterpret_cast<obs_source_t *>(_data);
			auto item = obs_scene_add(scene, source);
			obs_sceneitem_set_visible(item, true);
			struct vec2 scale;
			vec2_set(&scale, 0.2f, 0.2f);
			obs_sceneitem_set_scale(item, &scale);
		},
		source);
	obs_leave_graphics();
	obs_source_release(source);
}

void MainWindow::sourceTextClick()
{
	auto source = obs_source_create("text_gdiplus", "text_gdiplus test",
					NULL, nullptr);
	obs_data_t *settings = obs_data_create();
	obs_data_set_string(settings, "text", "cnnho live show");
	obs_source_update(source, settings);
	obs_data_release(settings);
	obs_enter_graphics();
	obs_scene_atomic_update(
		currentScene,
		[](void *_data, obs_scene_t *scene) {
			obs_source_t *source =
				reinterpret_cast<obs_source_t *>(_data);
			auto item = obs_scene_add(scene, source);
			UNUSED_PARAMETER(item);
		},
		source);
	obs_leave_graphics();
	obs_source_release(source);
}

void MainWindow::sourceAudioInput()
{
	obs_data_t *settings = obs_data_create();
	auto source = obs_source_create("wasapi_input_capture", "wasapi_input_capture test",
					NULL, nullptr);
	obs_data_set_string(
		settings, "device_id",
		"{0.0.1.00000000}.{2f445554-b4de-449d-996b-98f3e8ae55fb}");
	obs_source_update(source, settings);
	obs_data_release(settings);
	obs_enter_graphics();
	obs_scene_atomic_update(
		currentScene,
		[](void *_data, obs_scene_t *scene) {
			obs_source_t *source =
				reinterpret_cast<obs_source_t *>(_data);
			obs_scene_add(scene, source);
		},
		source);
	obs_leave_graphics();
	obs_source_release(source);
}

void MainWindow::sourceAudioOutput()
{
	obs_data_t *settings = obs_data_create();
	auto source = obs_source_create("wasapi_output_capture",
					"wasapi_output_capture test", NULL,
					nullptr);
	obs_data_set_string(settings, "device_id", "default");
	obs_source_update(source, settings);
	obs_data_release(settings);
	obs_enter_graphics();
	obs_scene_atomic_update(
		currentScene,
		[](void *_data, obs_scene_t *scene) {
			obs_source_t *source =
				reinterpret_cast<obs_source_t *>(_data);
			obs_scene_add(scene, source);
		},
		source);
	obs_leave_graphics();
	obs_source_release(source);
}

void MainWindow::startPushClick() {
	if (outputHandler->StreamingActive())
		return;

	if (!outputHandler->SetupStreaming(service)) {
		return;
	}

	//if (api)
	//	api->on_event(OBS_FRONTEND_EVENT_STREAMING_STARTING);

	//SaveProject();

	//ui->streamButton->setEnabled(false);
	//ui->streamButton->setChecked(false);
	//ui->streamButton->setText(QTStr("Basic.Main.Connecting"));

	//if (sysTrayStream) {
	//	sysTrayStream->setEnabled(false);
	//	sysTrayStream->setText(ui->streamButton->text());
	//}

	if (!outputHandler->StartStreaming(service)) {
		//DisplayStreamStartError();
		return;
	}

	//bool recordWhenStreaming = config_get_bool(
	//	GetGlobalConfig(), "BasicWindow", "RecordWhenStreaming");
	//if (recordWhenStreaming)
	//	StartRecording();

	//bool replayBufferWhileStreaming = config_get_bool(
	//	GetGlobalConfig(), "BasicWindow", "ReplayBufferWhileStreaming");
	//if (replayBufferWhileStreaming)
	//	StartReplayBuffer();
}

void MainWindow::stopPushClick() {
	outputHandler->StopStreaming();
}

void MainWindow::StreamDelayStarting(int sec)
{
	ui->labelStreamInfo->setText(QString("stream start after %1").arg(sec));
}

void MainWindow::StreamDelayStopping(int sec)
{
	ui->labelStreamInfo->setText(QString("stream stop after %1").arg(sec));
}

void MainWindow::StreamingStart()
{
	ui->labelStreamInfo->setText("stream started");
}

void MainWindow::StreamStopping()
{
	ui->labelStreamInfo->setText("stream stopping");
}

void MainWindow::StreamingStop(int errorcode, QString last_error) {
	ui->labelStreamInfo->setText(QString("stream stoped: %1, %2").arg(errorcode).arg(last_error));
}

void MainWindow::RenderMain(void *data, uint32_t cx, uint32_t cy)
{
	GS_DEBUG_MARKER_BEGIN(GS_DEBUG_COLOR_DEFAULT, "RenderMain");

	MainWindow *window = static_cast<MainWindow *>(data);
	obs_video_info ovi;

	obs_get_video_info(&ovi);

	window->previewCX = int(window->previewScale * float(ovi.base_width));
	window->previewCY = int(window->previewScale * float(ovi.base_height));

	gs_viewport_push();
	gs_projection_push();

	obs_display_t *display = window->ui->preview->GetDisplay();
	uint32_t width, height;
	obs_display_size(display, &width, &height);
	float right = float(width) - window->previewX;
	float bottom = float(height) - window->previewY;

	gs_ortho(-window->previewX, right, -window->previewY, bottom, -100.0f,
		 100.0f);

	// window->ui->preview->DrawOverflow();

	/* --------------------------------------- */

	gs_ortho(0.0f, float(ovi.base_width), 0.0f, float(ovi.base_height),
		 -100.0f, 100.0f);
	gs_set_viewport(window->previewX, window->previewY, window->previewCX,
			window->previewCY);

	obs_render_main_texture_src_color_only();
	gs_load_vertexbuffer(nullptr);

	/* --------------------------------------- */

	gs_ortho(-window->previewX, right, -window->previewY, bottom, -100.0f,
		 100.0f);
	gs_reset_viewport();

	window->ui->preview->DrawSceneEditing();

	/* --------------------------------------- */

	gs_projection_pop();
	gs_viewport_pop();

	GS_DEBUG_MARKER_END();

	UNUSED_PARAMETER(cx);
	UNUSED_PARAMETER(cy);
}

void MainWindow::InitPrimitives()
{
	ProfileScope("OBSBasic::InitPrimitives");

	obs_enter_graphics();

	gs_render_start(true);
	gs_vertex2f(0.0f, 0.0f);
	gs_vertex2f(0.0f, 1.0f);
	gs_vertex2f(1.0f, 0.0f);
	gs_vertex2f(1.0f, 1.0f);
	box = gs_render_save();

	gs_render_start(true);
	gs_vertex2f(0.0f, 0.0f);
	gs_vertex2f(0.0f, 1.0f);
	boxLeft = gs_render_save();

	gs_render_start(true);
	gs_vertex2f(0.0f, 0.0f);
	gs_vertex2f(1.0f, 0.0f);
	boxTop = gs_render_save();

	gs_render_start(true);
	gs_vertex2f(1.0f, 0.0f);
	gs_vertex2f(1.0f, 1.0f);
	boxRight = gs_render_save();

	gs_render_start(true);
	gs_vertex2f(0.0f, 1.0f);
	gs_vertex2f(1.0f, 1.0f);
	boxBottom = gs_render_save();

	gs_render_start(true);
	for (int i = 0; i <= 360; i += (360 / 20)) {
		float pos = RAD(float(i));
		gs_vertex2f(cosf(pos), sinf(pos));
	}
	circle = gs_render_save();

	obs_leave_graphics();
}

void MainWindow::InitService() {
	const char *type;
	obs_data_t *data =
		obs_data_create_from_json(QByteArray::fromBase64("ewogICAgInNldHRpbmdzIjogewogICAgICAgICJid3Rlc3QiOiBmYWxzZSwKICAgICAgICAia2V5IjogInN0cmVhbSIsCiAgICAgICAgInNlcnZlciI6ICJydG1wOi8vMTI3LjAuMC4xOjE5MzUvbGl2ZS8iLAogICAgICAgICJ1c2VfYXV0aCI6IGZhbHNlCiAgICB9LAogICAgInR5cGUiOiAicnRtcF9jdXN0b20iCn0=").toStdString().c_str());

	obs_data_set_default_string(data, "type", "rtmp_common");
	type = obs_data_get_string(data, "type");

	obs_data_t *settings = obs_data_get_obj(data, "settings");
	obs_data_t *hotkey_data = obs_data_get_obj(data, "hotkeys");

	service = obs_service_create(type, "default_service", settings,
				     hotkey_data);

	obs_service_release(service);
	obs_data_release(hotkey_data);
	obs_data_release(settings);
	obs_data_release(data);
}

void MainWindow::ResizePreview(uint32_t cx, uint32_t cy)
{
	QSize targetSize;
	bool isFixedScaling;
	obs_video_info ovi;

	/* resize preview panel to fix to the top section of the window */
	targetSize = GetPixelSize(ui->preview);

	isFixedScaling = ui->preview->IsFixedScaling();
	obs_get_video_info(&ovi);

	if (isFixedScaling) {
		previewScale = ui->preview->GetScalingAmount();
		GetCenterPosFromFixedScale(
			int(cx), int(cy),
			targetSize.width() - PREVIEW_EDGE_SIZE * 2,
			targetSize.height() - PREVIEW_EDGE_SIZE * 2, previewX,
			previewY, previewScale);
		previewX += ui->preview->GetScrollX();
		previewY += ui->preview->GetScrollY();

	} else {
		GetScaleAndCenterPos(int(cx), int(cy),
				     targetSize.width() - PREVIEW_EDGE_SIZE * 2,
				     targetSize.height() -
					     PREVIEW_EDGE_SIZE * 2,
				     previewX, previewY, previewScale);
	}

	previewX += float(PREVIEW_EDGE_SIZE);
	previewY += float(PREVIEW_EDGE_SIZE);
}
