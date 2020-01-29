/*
 * FileManager.h
 * Kamil Zacharczuk
 */
#ifndef SIMPLE_P2P_FILE_MANAGER_H
#define SIMPLE_P2P_FILE_MANAGER_H

#include <cstddef> // size_t
#include <string>
#include <fstream>
#include <mutex>
#include <condition_variable>

#include "LoggingModule.h"
#include "GeneralTypes.h"

namespace simpleP2P {
class CompleteResource;
/**
 * \brief Handles read/write to the files on disc.
 *
 * An API which provides:
 * - buffering contents of a requested segment of a specificated local file,
 * - storing a complete, downloaded file physically on the local disc.
 * Ensures synchronization of those operations.
 */
class FileManager {
public:
  FileManager(Logging_Module &lm);

  ~FileManager();
  /**
   * \brief Buffers the specificated segment of the specificated file in the char array.
   *
   * \warning Calling this function MUST be preceded by a successful call to read_lock() on the file
   * specified in resource_header equal to the one carried by the SegmentRequest.
   *
   * @param request Specifies file and its segment to buffer.
   * @param result The array to buffer the segment in. Its size must be greater or equal to the 3rd parameter!
   * @param requested_segment_size Size of the segment to buffer.
   * @return If the segment has been buffered successfully.
   */
  bool get_segment(const std::string file_name,
                   const Uint16 segment,
                   Uint8 *result,
                   const std::size_t requested_segment_size);

  /**
  * Stores the file contents in the physical file on disc.
  *
  * @param resource File to store on the disc.
  * The data will not be interpreted, so make sure it's complete and ready to store.
  */
  void store_resource(std::shared_ptr<CompleteResource> complete_resource);
  /**
   * Closes the file and unlocks it for writing.
   *
   * @param resource_header The file to be closed and unlocked.
   */

  /**
   * Opens a file for reading and prevents other threads from overwriting it until it's closed.
   *
   * @param resource_header The file to be opened and read-locked.
   * @return If the file has been opened successfully.
   */
  bool read_lock(const std::string file_name);

  /**
   * Closes a file which was opened for reading and enables other threads to write to it.
   *
   * \warning This function MUST be called after the reading has completed, so that the file can be overwritten in future.
   *
   * @param
   */
  void read_unlock(const std::string file_name);

private:
  /**
   * \brief A file opened for reading.
   *
   * Carries the file name which identifies the file and an open std::ifstream providing access to the file.
   */
  struct OpenFile {
    std::string file_name;
    std::ifstream stream;
  };

  std::vector<OpenFile *> rlocked_files;    //!< Files opened for reading, which also means - read-locked.
  std::vector<std::string> wlocked_files;  //!< Write-locked files.

  std::mutex rlmutex;                      //!< Mutex synchronizing access to the rlocked_files vector.
  std::mutex wlmutex;                      //!< Mutex synchronizing access to the wlocked_files vector.
  std::mutex condvmutex;                   //!< Mutex for the condition variable.
  std::condition_variable condv;           //!< Condition variable synchronizing access to local files.

  Logging_Module &logging_module;          //!< Logging_Module for logging events.

  /**
   * \brief Blocks other threads from reading/writing to the specified file.
   * \note Used behind the scenes by store_resource().
   * @param
   */
  void write_lock(const std::string file_name);

  /**
   * \brief Removes the lock performed in write_lock() from the specified file.
   * \note Used behind the scenes by store_resource().
   * @param
   */
  void write_unlock(const std::string file_name);
};
}

#endif // SIMPLE_P2P_FILE_MANAGER_H
