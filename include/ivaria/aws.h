#ifndef __IVARIA_AWS_H__
#define __IVARIA_AWS_H__

#include "csutil/scf.h"
#include "csgeom/csrect.h"
#include "csgeom/cspoint.h"
#include "iutil/string.h"

struct iAws;
struct iAwsSlot;
struct iAwsSink;
struct iAwsSource;
struct iAwsPrefManager;
struct iAwsSinkManager;

class  awsWindow;
class  awsComponent;
class  awsComponentNode;
class  awsComponentFactory;

struct  iGraphics2D;
struct  iGraphics3D;
struct  iEngine;
struct  iTextureManager;
struct  iObjectRegistry;
struct  iTextureHandle;
struct  iFontServer;
struct  iFont;
struct  iEvent;

const   bool aws_debug=false;  // set to true to turn on debugging printf's
       

SCF_VERSION (iAws, 0, 0, 1);

struct iAws : public iBase
{
public:  
  /// Get a pointer to the preference manager
  virtual iAwsPrefManager *GetPrefMgr()=0;

  /// Get a pointer to the sink manager
  virtual iAwsSinkManager *GetSinkMgr()=0;

  /// Set the preference manager used by the window system
  virtual void             SetPrefMgr(iAwsPrefManager *pmgr)=0;

  /// Allows a component to register itself for dynamic template instatiation via definition files.
  virtual void RegisterComponentFactory(awsComponentFactory *factory, char *name)=0;

  /// Get the top window
  virtual awsWindow *GetTopWindow()=0;

  /// Set the top window
  virtual void       SetTopWindow(awsWindow *win)=0;
  
  /// Causes the current view of the window system to be drawn to the given graphics device.
  virtual void       Print(iGraphics3D *g3d)=0;
  
  /// Redraw whatever portions of the screen need it.
  virtual void       Redraw()=0;

  /// Mark a region dirty
  virtual void       Mark(csRect &rect)=0;

  /// Mark a section of the screen clean.
  virtual void       Unmark(csRect &rect)=0;

  /// Tell the system to rebuild the update store
  virtual void       InvalidateUpdateStore()=0;

  /// Capture all mouse events until release is called, no matter where the mouse is
  virtual void       CaptureMouse()=0;

  /// Release the mouse events to go where they normally would.
  virtual void       ReleaseMouse()=0;

  /// Dispatches events to the proper components
  virtual bool HandleEvent(iEvent&)=0;
  
  /// Set the contexts however you want
  virtual void SetContext(iGraphics2D *g2d, iGraphics3D *g3d)=0;

  /// Set the context to the procedural texture
  virtual void SetDefaultContext(iEngine* engine, iTextureManager* txtmgr)=0;

  /// Get the iGraphics2D interface so that components can use it.
  virtual iGraphics2D *G2D()=0;

  /// Get the iGraphics3D interface so that components can use it.
  virtual iGraphics3D *G3D()=0; 
  
  /// Instantiates a window based on a window definition.
  virtual awsWindow *CreateWindowFrom(char *defname)=0;
  
};


SCF_VERSION (iAwsPrefManager, 0, 0, 1);

struct iAwsPrefManager : public iBase
{
public:

  /// Performs whatever initialization is needed
  virtual void Setup(iObjectRegistry *object_reg)=0;   

  /// Invokes the definition parser to load definition files
  virtual void Load(const char *def_file)=0;

  /// Maps a name to an id
  virtual unsigned long NameToId(char *name)=0;
    
  /// Select which skin is the default for components, the skin must be loaded.  True on success, false otherwise.
  virtual bool SelectDefaultSkin(char *skin_name)=0;

  /// Lookup the value of an int key by name (from the skin def)
  virtual bool LookupIntKey(char *name, int &val)=0; 

  /// Lookup the value of an int key by id (from the skin def)
  virtual bool LookupIntKey(unsigned long id, int &val)=0; 

  /// Lookup the value of a string key by name (from the skin def)
  virtual bool LookupStringKey(char *name, iString *&val)=0; 

  /// Lookup the value of a string key by id (from the skin def)
  virtual bool LookupStringKey(unsigned long id, iString *&val)=0; 

  /// Lookup the value of a rect key by name (from the skin def)
  virtual bool LookupRectKey(char *name, csRect &rect)=0; 

  /// Lookup the value of a rect key by id (from the skin def)
  virtual bool LookupRectKey(unsigned long id, csRect &rect)=0; 
  
  /// Lookup the value of an RGB key by name (from the skin def)
  virtual bool LookupRGBKey(char *name, unsigned char &red, unsigned char &green, unsigned char &blue)=0;
    
  /// Lookup the value of an RGB key by name (from the skin def)
  virtual bool LookupRGBKey(unsigned long id, unsigned char &red, unsigned char &green, unsigned char &blue)=0;

  /// Lookup the value of a point key by name (from the skin def)
  virtual bool LookupPointKey(char *name, csPoint &point)=0; 

  /// Lookup the value of a point key by id (from the skin def)
  virtual bool LookupPointKey(unsigned long id, csPoint &point)=0; 
  
  /// Get the an integer from a given component node
  virtual bool GetInt(awsComponentNode *node, char *name, int &val)=0;

  /// Get the a rect from a given component node
  virtual bool GetRect(awsComponentNode *node, char *name, csRect &rect)=0;

  /// Get the value of an integer from a given component node
  virtual bool GetString(awsComponentNode *node, char *name, iString *&val)=0;
  
  /// Find window definition and return the component node holding it, Null otherwise
  virtual awsComponentNode *FindWindowDef(char *name)=0;
  
  /// Sets the value of a color in the global AWS palette.
  virtual void SetColor(int index, int color)=0; 
    
  /// Gets the value of a color from the global AWS palette.
  virtual int  GetColor(int index)=0;

  /// Gets the current default font
  virtual iFont *GetDefaultFont()=0;

  /// Gets a font.  If it's not loaded, it will be.  Returns NULL on error.
  virtual iFont *GetFont(char *filename)=0;

  /// Gets a texture from the global AWS cache
  virtual iTextureHandle *GetTexture(char *name, char *filename=NULL)=0;

  /// Sets the texture manager that the preference manager uses
  virtual void SetTextureManager(iTextureManager *txtmgr)=0;

  /// Sets the font server that the preference manager uses
  virtual void SetFontServer(iFontServer *fntsvr)=0;
    
  /** Sets up the AWS palette so that the colors are valid reflections of
       user preferences.  Although SetColor can be used, it's recommended 
       that you do not.  Colors should always be a user preference, and 
       should be read from the window and skin definition files (as
       happens automatically normally. */
  virtual void SetupPalette()=0;

  /** Allows a component to specify it's own constant values for parsing. */
  virtual void RegisterConstant(char *name, int value)=0;

  /** Returns true if the constant has been registered, false otherwise.  */
  virtual bool ConstantExists(char *name)=0;

  /** Allows a component to retrieve the value of a constant, or the parser as well. */
  virtual int  GetConstantValue(char *name)=0;

};

SCF_VERSION (iAwsSinkManager, 0, 0, 1);

struct iAwsSinkManager : public iBase
{
  /// Registers a sink by name for lookup.
  virtual void RegisterSink(char *name, iAwsSink *sink)=0;

  /// Finds a sink by name for connection.
  virtual iAwsSink* FindSink(char *name)=0;
  
};

SCF_VERSION (iAwsSink, 0, 0, 1);

struct iAwsSink : public iBase
{
  /// Maps a trigger name to a trigger id
  virtual unsigned long GetTriggerID(char *name)=0;  

  /// Handles trigger events
  virtual void HandleTrigger(int trigger_id, iAwsSource &source)=0;

  /// A sink should call this to register trigger events
  virtual void RegisterTrigger(char *name, void (iBase::*Trigger)(iAwsSource &source))=0;
};


SCF_VERSION (iAwsSource, 0, 0, 1);

struct iAwsSource : public iBase
{      
  /// Registers a slot for any one of the signals defined by a source.  Each sources's signals exist in it's own namespace
  virtual bool RegisterSlot(iAwsSlot *slot, unsigned long signal)=0;

  /// Unregisters a slot for a signal.
  virtual bool UnregisterSlot(iAwsSlot *slot, unsigned long signal)=0;

  /// Broadcasts a signal to all slots that are interested.
  virtual void Broadcast(unsigned long signal)=0;
};


SCF_VERSION (iAwsSlot, 0, 0, 1);

struct iAwsSlot : public iBase
{
  /** This initializes a slot for a given component.  This slot may connect to any number of signal sources, and
   * all signals will be delivered back to this initialized slot.  Need only be intialized ONCE.
   */
  virtual void Initialize(iAwsSink *sink)=0;
  
  /** Connect sets us up to receive signals from some other component.  You can connect to as many different sources 
    and signals as you'd like.  You may connect to multiple signals from the same source.
  */
  virtual void Connect(iAwsSource &source, unsigned long signal, unsigned long trigger)=0;
  
  /**  Disconnects us from the specified source and signal.  This may happen automatically if the signal source
   *  goes away.  You will receive disconnect notification always (even if you request the disconnection.)
   */
  virtual void Disconnect(iAwsSource &source, unsigned long signal, unsigned long trigger)=0;

  /** Invoked by a source to emit the signal to this slot's sink.
   */
  virtual void Emit(iBase &source, unsigned long signal)=0;
};


SCF_VERSION (iAwsComponentFactory, 0, 0, 1);

struct iAwsComponentFactory : public iBase
{
  /// Returns a newly created component of the type this factory handles. 
  virtual awsComponent *Create()=0;

  /// Registers this factory with the window manager
  virtual void Register(char *type)=0;

  /// Registers constants for the parser so that we can construct right.
  virtual void RegisterConstant(char *name, int value)=0;
};


#endif // __IVARIA_AWS_H__
