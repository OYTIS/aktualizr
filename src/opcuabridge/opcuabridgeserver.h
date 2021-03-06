#ifndef OPCUABRIDGE_SERVER_H_
#define OPCUABRIDGE_SERVER_H_

#include "opcuabridge.h"

namespace opcuabridge {

class Server;

struct ServerModel {
  explicit ServerModel(UA_Server*);

  Configuration configuration_;

  VersionReport version_report_;

  MetadataFiles metadatafiles_;
  MetadataFile metadatafile_;

  FileList file_list_;
  FileData file_data_;

  friend class Server;

 private:
  std::size_t received_metadata_files_ = 0;
};

class ServerDelegate {
 public:
  virtual ~ServerDelegate() {}
  virtual void handleServerInitialized(ServerModel*) = 0;
  virtual void handleVersionReportRequested(ServerModel*) = 0;      // on requested by the client
  virtual void handleMetaDataFilesReceived(ServerModel*) = 0;       // after all metadata files recv.
  virtual void handleDirectoryFilesSynchronized(ServerModel*) = 0;  // when dir. sync. finished
};

class Server {
 public:
  Server(ServerDelegate*, uint16_t port = 4840);
  ~Server();

  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  bool run(volatile bool*);

 private:
  void onFileListUpdated(FileList*);
  void countReceivedMetadataFile(MetadataFile*);
  void onVersionReportRequested(VersionReport*);

  ServerModel* model_;

  ServerDelegate* delegate_;

  UA_Server* server_;
  UA_ServerConfig* server_config_;
};

}  // namespace opcuabridge

#endif  // OPCUABRIDGE_SERVER_H_
