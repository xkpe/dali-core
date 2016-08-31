/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// CLASS HEADER
#include <dali/internal/event/common/stage-impl.h>

// EXTERNAL INCLUDES
#include <algorithm>
#include <cmath>
#include <cstring> // for strcmp

// INTERNAL INCLUDES
#include <dali/integration-api/system-overlay.h>
#include <dali/internal/event/actors/layer-impl.h>
#include <dali/internal/event/actors/layer-list.h>
#include <dali/internal/event/actors/camera-actor-impl.h>
#include <dali/internal/event/common/system-overlay-impl.h>
#include <dali/internal/event/common/thread-local-storage.h>
#include <dali/internal/event/common/property-notification-manager.h>
#include <dali/internal/event/render-tasks/render-task-list-impl.h>
#include <dali/internal/update/nodes/node.h>
#include <dali/internal/event/common/object-registry-impl.h>
#include <dali/integration-api/platform-abstraction.h>
#include <dali/public-api/common/constants.h>
#include <dali/public-api/events/touch-data.h>
#include <dali/public-api/object/type-registry.h>
#include <dali/public-api/render-tasks/render-task-list.h>
#include <iostream> //TODOVR

#include <dali/integration-api/vr-engine.h>

using Dali::Internal::SceneGraph::Node;
using Dali::Integration::VrEngine;
using namespace Dali::Integration::Vr;

namespace Dali
{

namespace Internal
{

namespace
{

//TODOVR
enum Eye
{
  LEFT,
  RIGHT
};

//TODOVR: Sensor constraints
struct VrEyeConstraint
{
  VrEyeConstraint( Dali::Internal::Stage* stage )
    : stage( stage )
  {
  }

  void operator()( Quaternion& current, const PropertyInputContainer& inputs )
  {
    // TODO: get gyro data, update rotation
    // get data from sensor
    Dali::Integration::VrEngine* vrEngine( stage->GetVrEngine() );
    if( !vrEngine )
    {
      return;
    }

    VrEngineEyePose eyePose;
    eyePose.rotation = current;
    if( vrEngine->Get( VrEngine::EYE_CURRENT_POSE, &eyePose ) )
    {
      current = eyePose.rotation;
    }
    else
    {
      current = Quaternion( 1.0f, 0.0f, 0.0f, 0.0f );
    }
  }

  Dali::Internal::Stage* stage;
};

//TODOVR
const float DEFAULT_STEREO_BASE( 10.0f );
//const float DEFAULT_STEREO_BASE( 15.0f );

// Signals

const char* const SIGNAL_KEY_EVENT =                 "keyEvent";
const char* const SIGNAL_EVENT_PROCESSING_FINISHED = "eventProcessingFinished";
const char* const SIGNAL_TOUCHED =                   "touched";
const char* const SIGNAL_WHEEL_EVENT =               "wheelEvent";
const char* const SIGNAL_CONTEXT_LOST =              "contextLost";
const char* const SIGNAL_CONTEXT_REGAINED =          "contextRegained";
const char* const SIGNAL_SCENE_CREATED =             "sceneCreated";

TypeRegistration mType( typeid(Dali::Stage), typeid(Dali::BaseHandle), NULL );

SignalConnectorType signalConnector1( mType, SIGNAL_KEY_EVENT,                 &Stage::DoConnectSignal );
SignalConnectorType signalConnector2( mType, SIGNAL_EVENT_PROCESSING_FINISHED, &Stage::DoConnectSignal );
SignalConnectorType signalConnector3( mType, SIGNAL_TOUCHED,                   &Stage::DoConnectSignal );
SignalConnectorType signalConnector4( mType, SIGNAL_WHEEL_EVENT,               &Stage::DoConnectSignal );
SignalConnectorType signalConnector5( mType, SIGNAL_CONTEXT_LOST,              &Stage::DoConnectSignal );
SignalConnectorType signalConnector6( mType, SIGNAL_CONTEXT_REGAINED,          &Stage::DoConnectSignal );
SignalConnectorType signalConnector7( mType, SIGNAL_SCENE_CREATED,             &Stage::DoConnectSignal );

} // unnamed namespace

StagePtr Stage::New( AnimationPlaylist& playlist,
                     PropertyNotificationManager& propertyNotificationManager,
                     SceneGraph::UpdateManager& updateManager,
                     NotificationManager& notificationManager )
{
  return StagePtr( new Stage( playlist, propertyNotificationManager, updateManager, notificationManager ) );
}

void Stage::Initialize()
{
  mObjectRegistry = ObjectRegistry::New();

  // Create the ordered list of layers
  mLayerList = LayerList::New( mUpdateManager, false/*not system-level*/ );

  // The stage owns the default layer
  mRootLayer = Layer::NewRoot( *mLayerList, mUpdateManager, false/*not system-level*/ );
  mRootLayer->SetName("RootLayer");
  // The root layer needs to have a fixed resize policy (as opposed to the default USE_NATURAL_SIZE).
  // This stops actors parented to the stage having their relayout requests propagating
  // up to the root layer, and down through other children unnecessarily.
  mRootLayer->SetResizePolicy( ResizePolicy::FIXED, Dimension::ALL_DIMENSIONS );

  // Create the default camera actor first; this is needed by the RenderTaskList
  CreateDefaultCameraActor();

  // Create the list of render-tasks
  mRenderTaskList = RenderTaskList::New( *this, *this, false/*not system-level*/ );

  // Create the default render-task
  Dali::RenderTask defaultRenderTask = mRenderTaskList->CreateTask();
}

void Stage::Uninitialize()
{
  // Remove actors added to SystemOverlay
  delete mSystemOverlay;
  mSystemOverlay = NULL;

  if( mDefaultCamera )
  {
    // its enough to release the handle so the object is released
    // don't need to remove it from root actor as root actor will delete the object
    mDefaultCamera.Reset();
  }

  if( mRootLayer )
  {
    // we are closing down so just delete the root, no point emit disconnect
    // signals or send messages to update
    mRootLayer.Reset();
  }
}

StagePtr Stage::GetCurrent()
{
  StagePtr stage( NULL );
  // no checking in this version
  ThreadLocalStorage* tls = ThreadLocalStorage::GetInternal();
  if( tls )
  {
    stage = tls->GetCurrentStage();
  }
  return stage;
}

bool Stage::IsInstalled()
{
  return ThreadLocalStorage::Created();
}

ObjectRegistry& Stage::GetObjectRegistry()
{
  return *mObjectRegistry;
}

void Stage::RegisterObject( Dali::BaseObject* object )
{
  mObjectRegistry->RegisterObject( object );
}

void Stage::UnregisterObject( Dali::BaseObject* object )
{
  mObjectRegistry->UnregisterObject( object );
}

Layer& Stage::GetRootActor()
{
  return *mRootLayer;
}

AnimationPlaylist& Stage::GetAnimationPlaylist()
{
  return mAnimationPlaylist;
}

PropertyNotificationManager& Stage::GetPropertyNotificationManager()
{
  return mPropertyNotificationManager;
}

void Stage::Add( Actor& actor )
{
  mRootLayer->Add( actor );
}

void Stage::Remove( Actor& actor )
{
  mRootLayer->Remove( actor );
}

void Stage::SetSize(float width, float height)
{
  // Internally we want to report the actual size of the stage.
  mSize.width  = width;
  mSize.height = height;

  // Calculates the aspect ratio, near and far clipping planes, field of view and camera Z position.
  mDefaultCamera->SetPerspectiveProjection( mSize );

  // The depth of the stage gets set to the maximun of these values
  mRootLayer->SetSize( mSize );

  // Repeat for SystemOverlay actors
  if( mSystemOverlay )
  {
    mSystemOverlay->GetImpl()->SetSize( mSize.width, mSize.height );
  }

  SetDefaultSurfaceRectMessage( mUpdateManager, Rect<int>( 0, 0, width, height ) );

  // if single render task to screen then set its viewport parameters
  if( 1 == mRenderTaskList->GetTaskCount() )
  {
    Dali::RenderTask mDefaultRenderTask = mRenderTaskList->GetTask(0);

    if(!mDefaultRenderTask.GetTargetFrameBuffer())
    {
      mDefaultRenderTask.SetViewport( Viewport(0, 0, width, height) );
    }
  }

}

Vector2 Stage::GetSize() const
{
  return mSize;
}

RenderTaskList& Stage::GetRenderTaskList() const
{
  return *mRenderTaskList;
}

void Stage::CreateDefaultCameraActor()
{
  // The default camera attributes and position is such that
  // children of the default layer, can be positioned at (0,0) and
  // be at the top-left of the viewport.
  mDefaultCamera = CameraActor::New( Size::ZERO );
  mDefaultCamera->SetParentOrigin( ParentOrigin::CENTER );
  mDefaultCamera->SetAnchorPoint( AnchorPoint::CENTER );
  Add( *( mDefaultCamera.Get() ) );
}

//TODOVR
Dali::CameraActor Stage::GetCameraActor()
{
  return Dali::CameraActor( mDefaultCamera.Get() );
}

Actor& Stage::GetDefaultRootActor()
{
  return *mRootLayer;
}

CameraActor& Stage::GetDefaultCameraActor()
{
  return *mDefaultCamera;
}

unsigned int Stage::GetLayerCount() const
{
  return mLayerList->GetLayerCount();
}

Dali::Layer Stage::GetLayer( unsigned int depth ) const
{
  return Dali::Layer(mLayerList->GetLayer( depth ));
}

Dali::Layer Stage::GetRootLayer() const
{
  return Dali::Layer( mRootLayer.Get() );
}

LayerList& Stage::GetLayerList()
{
  return *mLayerList;
}

Integration::SystemOverlay& Stage::GetSystemOverlay()
{
  // Lazily create system-level if requested
  if( !mSystemOverlay )
  {
    mSystemOverlay = new Integration::SystemOverlay( SystemOverlay::New( *this ) );
    DALI_ASSERT_ALWAYS( NULL != mSystemOverlay && "Failed to create system overlay" );

    mSystemOverlay->GetImpl()->SetSize( mSize.width, mSize.height );
  }

  return *mSystemOverlay;
}

SystemOverlay* Stage::GetSystemOverlayInternal()
{
  SystemOverlay* overlay( NULL );

  if( mSystemOverlay )
  {
    overlay = mSystemOverlay->GetImpl();
  }

  return overlay;
}

void Stage::UpdateCameras()
{
  switch( mViewMode )
  {
    case MONO:
    {
      // Delete extra stereoscopic render tasks and cameras
      mRenderTaskList->RemoveTask( mStereoInfo[LEFT].renderTask );
      mDefaultCamera->Remove( *mStereoInfo[LEFT].camera.Get() );
      mStereoInfo[LEFT].renderTask.Reset();
      mStereoInfo[LEFT].camera.Reset();
      mRenderTaskList->RemoveTask( mStereoInfo[RIGHT].renderTask );
      mDefaultCamera->Remove( *mStereoInfo[RIGHT].camera.Get() );
      mStereoInfo[RIGHT].renderTask.Reset();
      mStereoInfo[RIGHT].camera.Reset();
      mDefaultCamera->SetOrientation( Dali::ANGLE_0, Vector3::YAXIS );
      mDefaultCamera->SetType( Dali::Camera::LOOK_AT_TARGET );
      mRenderTaskList->GetTask( 0 ).SetSourceActor( Dali::Layer( mRootLayer.Get() ) );

      break;
    }

    case STEREO_HORIZONTAL:
    {
      // Stereo mode with horizontal split is for landscape mode. That's the reason for the cameras being rotated
      // Top camera renders the scene as seen from the right eye and bottom camera as seen from left.

      // Calculate aspect ratio
      float aspect = mSize.width / ( mSize.height * 0.5f );
      mStereoInfo[LEFT].camera->SetAspectRatio( aspect );
      mStereoInfo[LEFT].camera->SetOrientation( -Dali::ANGLE_90, Vector3::ZAXIS );
      mStereoInfo[LEFT].renderTask.SetViewport( Viewport(0, mSize.height * 0.5f, mSize.width, mSize.height * 0.5f ) );
      mStereoInfo[RIGHT].camera->SetAspectRatio( aspect );
      mStereoInfo[RIGHT].camera->SetOrientation( -Dali::ANGLE_90, Vector3::ZAXIS );
      mStereoInfo[RIGHT].renderTask.SetViewport( Viewport( 0, 0, mSize.width, mSize.height * 0.5f ) );
      break;
    }

    case STEREO_VERTICAL:
    {
      // Recalculate fov based on viewport size
      const float fov = 2.0f * std::atan( mSize.y / ( 2.0f * std::max( mSize.x * 0.5f, mSize.y ) ) );
      mStereoInfo[LEFT].camera->SetFieldOfView( fov );
      mStereoInfo[LEFT].camera->SetOrientation( Dali::ANGLE_0, Vector3::ZAXIS );
      mStereoInfo[LEFT].renderTask.SetViewport( Viewport(0, 0, mSize.width * 0.5f, mSize.height ) );
      mStereoInfo[RIGHT].camera->SetFieldOfView( fov );
      mStereoInfo[RIGHT].camera->SetOrientation( Dali::ANGLE_0, Vector3::ZAXIS );
      mStereoInfo[RIGHT].renderTask.SetViewport( Viewport( mSize.width * 0.5f, 0, mSize.width * 0.5f, mSize.height ) );
      break;
    }

    case VR:
    {
      const float pixelAspect = GetDpi().y / GetDpi().x;
      float stereoBase( 0.05f );
      int L = LEFT;
      int R = RIGHT;
      if( mSize.width > mSize.height )
      {
        // Stereo mode with horizontal split is for landscape mode. That's the reason for the cameras being rotated
        // Top camera renders the scene as seen from the right eye and bottom camera as seen from left.
        // Calculate separation in pixels along vertical axis ( mStereoBase is defined in millimetres )

        // Precalculations:
        const float sizeX = mSize.y / pixelAspect;
        const float viewPortWidth = mSize.x / 2.0f;
        const float cameraAspect = pixelAspect / ( sizeX / viewPortWidth );

        // We should not alter the frustums based on eye separation.
        mStereoInfo[LEFT].camera->SetPerspectiveProjection( Size( sizeX, mSize.height ), Vector2::ZERO );
        mStereoInfo[LEFT].camera->SetAspectRatio( cameraAspect );
        mStereoInfo[LEFT].camera->SetOrientation( Dali::ANGLE_0, Vector3::ZAXIS );
        mStereoInfo[LEFT].renderTask.SetViewport( Viewport( 0, 0, viewPortWidth, mSize.height ) );
        mStereoInfo[RIGHT].camera->SetPerspectiveProjection( Size( sizeX, mSize.height ), Vector2::ZERO );
        mStereoInfo[RIGHT].camera->SetAspectRatio( cameraAspect );
        mStereoInfo[RIGHT].camera->SetOrientation( Dali::ANGLE_0, Vector3::ZAXIS );
        mStereoInfo[RIGHT].renderTask.SetViewport( Viewport( viewPortWidth, 0, viewPortWidth, mSize.height ) );
      }
      else
      {

#if 0
        // Portrait aspect - default to VR on device.
        // Precalculations:
        const float sizeY = mSize.x * pixelAspect;
        const float viewPortHeight = mSize.y / 2.0f;
        const float cameraAspect = pixelAspect * ( sizeY / viewPortHeight );
        const float near = 100.0f;
        const float far = 5000.0f;
        // TODOVR: Base this off actual FoV values (EG. 96 degrees for GearVR)
        // Recalculate fov based on viewport size.
        const float fov = Radian( Degree(60) );//std::atan( viewPortHeight / ( 2.0f * mSize.width ) );

        mStereoInfo[LEFT].camera->SetPerspectiveProjectionFovY( fov, cameraAspect, near, far, Vector2::ZERO );
        mStereoInfo[LEFT].camera->SetAspectRatio( cameraAspect );
        mStereoInfo[LEFT].camera->SetOrientation( -Dali::ANGLE_90, Vector3::ZAXIS );
        mStereoInfo[LEFT].camera->SetFieldOfView( fov );
        mStereoInfo[LEFT].renderTask.SetViewport( Viewport( 0, viewPortHeight, mSize.width, viewPortHeight ) );

        mStereoInfo[RIGHT].camera->SetPerspectiveProjectionFovY( fov, cameraAspect, near, far, Vector2::ZERO );
        mStereoInfo[RIGHT].camera->SetAspectRatio( cameraAspect );
        mStereoInfo[RIGHT].camera->SetOrientation( -Dali::ANGLE_90, Vector3::ZAXIS );
        mStereoInfo[RIGHT].camera->SetFieldOfView( fov );
        mStereoInfo[RIGHT].renderTask.SetViewport( Viewport( 0, 0, mSize.width, viewPortHeight ) );

        mDefaultCamera->SetPosition( 0.0f, 100.0f );
#endif

        // Portrait aspect - default to VR on device.
        // Precalculations:
        const float sizeY = mSize.x * pixelAspect;
        const float viewPortHeight = mSize.y / 2.0f;
        const float cameraAspect = pixelAspect * ( sizeY / viewPortHeight );

        // TODOVR: Base this off actual FoV values (EG. 96 degrees for GearVR)
        // Recalculate fov based on viewport size.
        float fov = std::atan( viewPortHeight / ( 2.0f * mSize.width ) );
        const float far = 300.0f;
        float IPD = 0.0635f;
        stereoBase = -IPD*0.5f;
        const float near = IPD+0.1f;
        //stereoBase = 0.0f;
        fov = Radian( Degree(106) );

        mStereoInfo[L].camera->SetPerspectiveProjectionFovY( fov, cameraAspect, near, far, Vector2( +stereoBase, 0.0f ) );
        //mStereoInfo[LEFT].camera->SetPerspectiveProjection( Size( mSize.width, sizeY ), Vector2::ZERO );
        //mStereoInfo[LEFT].camera->SetPerspectiveProjection( Size( mSize.width, sizeY ), Vector2( 0.0f, stereoBase ) );
        //mStereoInfo[LEFT].camera->SetAspectRatio( cameraAspect );
        mStereoInfo[L].camera->SetOrientation( Dali::ANGLE_90, Vector3::ZAXIS );
        //mStereoInfo[LEFT].camera->SetFieldOfView( fov );
        mStereoInfo[L].renderTask.SetViewport( Viewport( 0, 0, 1024, 1024 ) );

        //mStereoInfo[RIGHT].camera->SetPerspectiveProjection( Size( mSize.width, sizeY ), Vector2::ZERO );
        //mStereoInfo[RIGHT].camera->SetPerspectiveProjection( Size( mSize.width, sizeY ), Vector2( 0.0, -stereoBase ) );
        mStereoInfo[R].camera->SetPerspectiveProjectionFovY( fov, cameraAspect, near, far, Vector2( -stereoBase, 0.0f ) );
        //mStereoInfo[RIGHT].camera->SetAspectRatio( cameraAspect );
        mStereoInfo[R].camera->SetOrientation( Dali::ANGLE_90, Vector3::ZAXIS );
        //mStereoInfo[RIGHT].camera->SetFieldOfView( fov );
        mStereoInfo[R].renderTask.SetViewport( Viewport( 0, 0, 1024, 1024 ) );

      }

      mVRGyroEyeConstraint = Constraint::New<Quaternion>( mDefaultCamera.Get(),
                                                        Dali::Actor::Property::ORIENTATION,
                                                        VrEyeConstraint( this ));
      mVRGyroEyeConstraint.Apply();
      Dali::Actor actor( mDefaultCamera.Get() );
      Quaternion quaternion( Radian(Degree(0)), Vector3( 0.0f, 0.0f, 0.0f ));
      mVRDefaultCameraAnimation = Dali::Animation::New( 1.0f );
      mVRDefaultCameraAnimation.AnimateBy( Property( actor, Dali::Actor::Property::ORIENTATION ), quaternion, AlphaFunction::LINEAR );
      mVRDefaultCameraAnimation.SetLooping( true );
      mVRDefaultCameraAnimation.Play();

      mStereoInfo[L].camera->SetType( Camera::VR_EYE_LEFT );
      mStereoInfo[R].camera->SetType( Camera::VR_EYE_RIGHT );
      // Same settings regardless of orientation:
      //stereoBase = 0;
      mStereoInfo[L].camera->SetPosition( Vector3( 0.0f, -stereoBase, 0.0f ) );
      mStereoInfo[R].camera->SetPosition( Vector3( 0.f, +stereoBase, 0.0f ) );
      break;
    }

    case STEREO_INTERLACED:
    {
      break;
    }
  }
}

void Stage::SetViewMode( ViewMode viewMode )
{
  if( mViewMode != viewMode )
  {
    DALI_LOG_INFO( Debug::Filter::gActor, Debug::Concise, "View mode changed from %d to %d\n", mViewMode, viewMode);

    // Clean up previous mode.
    if( mViewMode == MONO )
    {
      mDefaultCamera->SetOrientation( Dali::ANGLE_180, Vector3::YAXIS );
      mRenderTaskList->GetTask(0).SetSourceActor( Dali::Actor() );

      // Create camera and RenderTask for left eye
      mStereoInfo[LEFT].camera = CameraActor::New( Size::ZERO );
      mStereoInfo[LEFT].camera->SetParentOrigin( ParentOrigin::CENTER );
      mDefaultCamera->Add( *mStereoInfo[LEFT].camera.Get() );
      mStereoInfo[LEFT].renderTask = mRenderTaskList->CreateTask();
      mStereoInfo[LEFT].renderTask.SetClearColor( Vector4( 1.0f, 0.0f, 0.0f, 1.0f ) );
      mStereoInfo[LEFT].renderTask.SetCameraActor( Dali::CameraActor( mStereoInfo[LEFT].camera.Get() ) );
      mStereoInfo[LEFT].camera->SetType( Dali::Camera::FREE_LOOK );

      // Create camera and RenderTask for right eye
      mStereoInfo[RIGHT].camera = CameraActor::New( Size::ZERO );
      mStereoInfo[RIGHT].camera->SetParentOrigin( ParentOrigin::CENTER );
      mDefaultCamera->Add( *mStereoInfo[RIGHT].camera.Get() );
      mStereoInfo[RIGHT].renderTask = mRenderTaskList->CreateTask();
      mStereoInfo[RIGHT].renderTask.SetClearColor( Vector4( 1.0f, 0.0f, 0.0f, 1.0f ) );
      mStereoInfo[RIGHT].renderTask.SetCameraActor( Dali::CameraActor( mStereoInfo[RIGHT].camera.Get() ) );
      mStereoInfo[RIGHT].camera->SetType( Dali::Camera::FREE_LOOK );
    }

    // Save new mode
    mViewMode = viewMode;

    UpdateCameras();
  }

  //TODOVR: Animate the look angle for testing.
#if 0
  Quaternion q = mDefaultCamera->GetCurrentOrientation();

  float duration = 3.0f;
  mCamAnim = Dali::Animation::New( duration );

  Dali::CameraActor a = Dali::CameraActor( ( mDefaultCamera.Get() ) );

  float lookAngle = 20.0f;
  mDefaultCamera->SetOrientation( Degree( ( 180.0f - ( lookAngle / 2.0f ) ) ), Vector3::YAXIS );

  mCamAnim.AnimateBy( Dali::Property( a, Dali::Actor::Property::ORIENTATION ), Quaternion( Radian( Degree( lookAngle ) ), Vector3::YAXIS ), AlphaFunction::EASE_IN_OUT, TimePeriod( 0.0f, duration / 2.0f ) );
  mCamAnim.AnimateBy( Dali::Property( a, Dali::Actor::Property::ORIENTATION ), Quaternion( Radian( Degree( -lookAngle ) ), Vector3::YAXIS ), AlphaFunction::EASE_IN_OUT, TimePeriod( duration / 2.0f, duration / 2.0f )  );
  mCamAnim.SetLooping( true );

  mDefaultCamera->SetOrientation( Radian( Math::PI * 0.95f ), Vector3::YAXIS );
#endif
}

ViewMode Stage::GetViewMode() const
{
  return mViewMode;
}

void Stage::SetStereoBase( float stereoBase )
{
  if( ! Equals( mStereoBase, stereoBase ) )
  {
    DALI_LOG_INFO( Debug::Filter::gActor, Debug::Concise, "old( %.2f) new(%.2f)\n", mStereoBase, stereoBase );
    mStereoBase = stereoBase;

    UpdateCameras();
  }
}

float Stage::GetStereoBase() const
{
  return mStereoBase;
}

void Stage::SetBackgroundColor(Vector4 color)
{
  // Cache for public GetBackgroundColor()
  mBackgroundColor = color;

  // Send message to change color in next frame
  SetBackgroundColorMessage( mUpdateManager, color );
}

Vector4 Stage::GetBackgroundColor() const
{
  return mBackgroundColor;
}

Vector2 Stage::GetDpi() const
{
  return mDpi;
}

void Stage::SetDpi(Vector2 dpi)
{
  mDpi = dpi;
}

void Stage::KeepRendering( float durationSeconds )
{
  // Send message to keep rendering
  KeepRenderingMessage( mUpdateManager, durationSeconds );
}

bool Stage::DoConnectSignal( BaseObject* object, ConnectionTrackerInterface* tracker, const std::string& signalName, FunctorDelegate* functor )
{
  bool connected( true );
  Stage* stage = dynamic_cast<Stage*>(object);

  if( 0 == strcmp( signalName.c_str(), SIGNAL_KEY_EVENT ) )
  {
    stage->KeyEventSignal().Connect( tracker, functor );
  }
  else if( 0 == strcmp( signalName.c_str(), SIGNAL_EVENT_PROCESSING_FINISHED ) )
  {
    stage->EventProcessingFinishedSignal().Connect( tracker, functor );
  }
  else if( 0 == strcmp( signalName.c_str(), SIGNAL_TOUCHED ) )
  {
    stage->TouchedSignal().Connect( tracker, functor );
  }
  else if( 0 == strcmp( signalName.c_str(), SIGNAL_WHEEL_EVENT ) )
  {
    stage->WheelEventSignal().Connect( tracker, functor );
  }
  else if( 0 == strcmp( signalName.c_str(), SIGNAL_CONTEXT_LOST ) )
  {
    stage->ContextLostSignal().Connect( tracker, functor );
  }
  else if( 0 == strcmp( signalName.c_str(), SIGNAL_CONTEXT_REGAINED ) )
  {
    stage->ContextRegainedSignal().Connect( tracker, functor );
  }
  else if( 0 == strcmp( signalName.c_str(), SIGNAL_SCENE_CREATED ) )
  {
    stage->SceneCreatedSignal().Connect( tracker, functor );
  }
  else
  {
    // signalName does not match any signal
    connected = false;
  }

  return connected;
}

void Stage::EmitKeyEventSignal(const KeyEvent& event)
{
  // Emit the key event signal when no actor in the stage has gained the key input focus

  mKeyEventSignal.Emit( event );
}

void Stage::EmitEventProcessingFinishedSignal()
{
   mEventProcessingFinishedSignal.Emit();
}

void Stage::EmitTouchedSignal( const TouchEvent& touchEvent, const Dali::TouchData& touch )
{
  mTouchedSignal.Emit( touchEvent );
  mTouchSignal.Emit( touch );
}

void Stage::EmitWheelEventSignal(const WheelEvent& event)
{
  // Emit the wheel event signal when no actor in the stage has gained the wheel input focus

  mWheelEventSignal.Emit( event );
}

void Stage::EmitSceneCreatedSignal()
{
  mSceneCreatedSignal.Emit();
}

Dali::Stage::KeyEventSignalType& Stage::KeyEventSignal()
{
  return mKeyEventSignal;
}

Dali::Stage::EventProcessingFinishedSignalType& Stage::EventProcessingFinishedSignal()
{
  return mEventProcessingFinishedSignal;
}

Dali::Stage::TouchedSignalType& Stage::TouchedSignal()
{
  DALI_LOG_WARNING( "Deprecated. Use TouchSignal() instead.\n" );
  return mTouchedSignal;
}

Dali::Stage::TouchSignalType& Stage::TouchSignal()
{
  return mTouchSignal;
}

Dali::Stage::WheelEventSignalType& Stage::WheelEventSignal()
{
  return mWheelEventSignal;
}

Dali::Stage::ContextStatusSignal& Stage::ContextLostSignal()
{
  return mContextLostSignal;
}

Dali::Stage::ContextStatusSignal& Stage::ContextRegainedSignal()
{
  return mContextRegainedSignal;
}

Dali::Stage::SceneCreatedSignalType& Stage::SceneCreatedSignal()
{
  return mSceneCreatedSignal;
}

void Stage::NotifyContextLost()
{
  mContextLostSignal.Emit();
}

void Stage::NotifyContextRegained()
{
  mContextRegainedSignal.Emit();
}

Stage::Stage( AnimationPlaylist& playlist,
              PropertyNotificationManager& propertyNotificationManager,
              SceneGraph::UpdateManager& updateManager,
              NotificationManager& notificationManager )
: mAnimationPlaylist( playlist ),
  mPropertyNotificationManager(propertyNotificationManager),
  mUpdateManager(updateManager),
  mNotificationManager(notificationManager),
  mSize(Vector2::ZERO),
  mBackgroundColor(Dali::Stage::DEFAULT_BACKGROUND_COLOR),
  mViewMode( MONO ),
  mStereoBase( DEFAULT_STEREO_BASE ),
  mSystemOverlay(NULL)
{
}

SceneGraph::UpdateManager& Stage::GetUpdateManager()
{
  return mUpdateManager;
}

unsigned int* Stage::ReserveMessageSlot( std::size_t size, bool updateScene )
{
  return mUpdateManager.ReserveMessageSlot( size, updateScene );
}

BufferIndex Stage::GetEventBufferIndex() const
{
  return mUpdateManager.GetEventBufferIndex();
}

Stage::~Stage()
{
  delete mSystemOverlay;

  mObjectRegistry.Reset();
}

} // namespace Internal

} // namespace Dali
