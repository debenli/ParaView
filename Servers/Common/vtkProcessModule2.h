/*=========================================================================

  Program:   ParaView
  Module:    vtkProcessModule2.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkProcessModule2 - process initialization and management core for
// ParaView processes.
// vtkProcessModule2 is the process initialization and session management core
// for ParaView processes.
#ifndef __vtkProcessModule2_h
#define __vtkProcessModule2_h

#include "vtkObject.h"
#include "vtkSmartPointer.h" // needed for vtkSmartPointer.

class vtkMultiProcessController;
class vtkNetworkAccessManager;
class vtkPVOptions;
class vtkSession;
class vtkSessionIterator;

class vtkProcessModule2 : public vtkObject
{
public:
  static vtkProcessModule2* New();
  vtkTypeMacro(vtkProcessModule2, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  enum ProcessTypes
    {
    PROCESS_CLIENT, /* Capable of connecting to remote server or self.
               Cannot run in Parallel */
    PROCESS_SERVER, /* data-server+render-server */
    PROCESS_DATA_SERVER, /* data-server */
    PROCESS_RENDER_SERVER, /* render-server */
    PROCESS_BATCH, /* Capable of running in parallel with root node acting as client.
              Cannot connect to remote server */
    PROCESS_SYMMETRIC_BATCH, /* Same as BATCH except, every node acts as a client */

    PROCESS_INVALID=-1,
    };

  // Description:
  // These flags are used to specify destination servers for the
  // SendStream function.
  // FIXME: These flags should simply move to PVSession. They don't make sense
  // in non-PVSessions since the processes may have different roles in that
  // case.
  enum ServerFlags
    {
    DATA_SERVER = 0x01,
    DATA_SERVER_ROOT = 0x02,
    RENDER_SERVER = 0x04,
    RENDER_SERVER_ROOT = 0x08,
    SERVERS = DATA_SERVER | RENDER_SERVER,
    CLIENT = 0x10,
    CLIENT_AND_SERVERS = DATA_SERVER | CLIENT | RENDER_SERVER
    };

  static ProcessTypes GetProcessType()
    { return vtkProcessModule2::ProcessType; }

  //********** PROCESS INITIALIZATION/CLEANUP API *****************************

  // Description:
  // Initializes the process and the ProcessModule.
  // The function is to initialize MPI if applicable
  // for the process and setup some environment e.g. DISPLAY.
  // Initializes the ProcessModule.
  // for the process and setup some environment e.g. DISPLAY.
  static bool Initialize(ProcessTypes type, int& argc, char** &argv);

  // Description:
  // Finalizes and cleans up the process.
  static bool Finalize();

  //********** SESSION MANAGEMENT API *****************************

  // Description:
  // Registers a new session. A new ID is assigned for the session and
  // that ID is returned. The ID can be used in future to access this
  // session.
  vtkIdType RegisterSession(vtkSession*);

  // Description:
  // Unregister a session given its ID. This is the same ID that is returned
  // when the session was registered. Returns true is the session was
  // unregistered. Unregistering a session implies that the ProcessModule
  // will no longer monitor communication on the sockets, if any, in the
  // session.
  bool UnRegisterSession(vtkIdType sessionID);
  bool UnRegisterSession(vtkSession* session);

  // Description:
  // Returns the session associated with a given ID.
  vtkSession* GetSession(vtkIdType);

  // Description:
  // Returns a new session iterator that can be used to iterate over the
  // registered sessions.
  vtkSessionIterator* NewSessionIterator();

  // Description:
  // Whenever any session is processing some message, it typically marks itself
  // active with the process module. The active session can be accessed using
  // this method.
  vtkSession* GetActiveSession();

  // Description:
  // This is a convenience method that either returns the active session, if
  // present, otherwise the first session. Don't use this for new API. This is
  // provided for some old api.
  vtkSession* GetSession();

  //********** ACCESSORS FOR VARIOUS HELPERS *****************************

  // Description:
  // Provides access to the global ProcessModule. This method can only be called
  // after Initialize().
  static vtkProcessModule2* GetProcessModule()
    { return vtkProcessModule2::Singleton.GetPointer(); }

  // Description:
  // Set/Get the application command line options object.
  // Note that this has to be explicitly set. vtkProcessModule2::Initialize()
  // does not initialize the vtkPVOptions.
  vtkGetObjectMacro(Options, vtkPVOptions);
  void SetOptions(vtkPVOptions* op);


  //********** ACCESSORS FOR VARIOUS HELPERS *****************************

  // Description::
  // Get/Set the network access manager. vtkNetworkAccessManager encapsulates
  // the setup of interprocess communication channels. By default a
  // vtkTCPNetworkAccessManager is setup. If you want to change the network
  // access manager, it should be done only when no sessions are present.
  // Ideally, you want to do that during the initialization of the process
  // itself.
  vtkGetObjectMacro(NetworkAccessManager, vtkNetworkAccessManager);
  void SetNetworkAccessManager(vtkNetworkAccessManager*);

  // Description:
  // Provides access to the global MPI controller, if any. Same can be obtained
  // using vtkMultiProcessController::GetGlobalController();
  vtkMultiProcessController* GetGlobalController();

  // Description:
  // Returns the number of processes in this process group.
  int GetNumberOfLocalPartitions();

  // Description:
  // Returns the local process id.
  int GetPartitionId();

  // Description:
  // Set/Get whether to report errors from the Interpreter.
  vtkGetMacro(ReportInterpreterErrors, bool);
  vtkSetMacro(ReportInterpreterErrors, bool);
  vtkBooleanMacro(ReportInterpreterErrors, bool);

//BTX
protected:
  vtkProcessModule2();
  ~vtkProcessModule2();

  // Description:
  // Push/Pop the active session.
  void PushActiveSession(vtkSession*);
  void PopActiveSession(vtkSession*);

  // Description:
  // Marking vtkSession as friend since it needs access to
  // PushActiveSession/PopActiveSession.
  friend class vtkSession;

  vtkNetworkAccessManager* NetworkAccessManager;
  vtkPVOptions* Options;

  // Description:
  // Used to keep track of maximum session used. Only used to ensure that no
  // session id is ever repeated.
  vtkIdType MaxSessionId;

  class vtkInternals;
  vtkInternals* Internals;

  // vtkSessionIterator needs access to vtkInternals to be able to iterate over
  // the sessions efficiently.
  friend class vtkSessionIterator;

  bool ReportInterpreterErrors;
private:
  vtkProcessModule2(const vtkProcessModule2&); // Not implemented.
  void operator=(const vtkProcessModule2&); // Not implemented.

  static ProcessTypes ProcessType;

  // Set to true in Initialize if Finalize() should cleanup MPI.
  static bool FinalizeMPI;

  static vtkSmartPointer<vtkProcessModule2> Singleton;
  static vtkSmartPointer<vtkMultiProcessController> GlobalController;
//ETX
};

#endif //__vtkProcessModule2_h
