/**
 * @file filewriter_impl.hpp
 * @author Jeffin Sam
 */
#ifndef MLBOARD_FILE_WRITER_IMPL_HPP
#define MLBOARD_FILE_WRITER_IMPL_HPP

#include "filewriter.hpp"

namespace mlboard {


fileWriter::fileWriter(std::string logdir,
                       int maxQueueSize,
                        std::size_t flushsec)
{
  const auto p1 = std::chrono::system_clock::now();
  std::string currentTime =
      std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
      p1.time_since_epoch()).count());
  this->logdir = logdir + "/events.out.tfevents." + currentTime + ".v2";
  close_ = true;
  this->flushsec = flushsec;
  size_t &maxSize = this->q.MaxSize();
  maxSize = maxQueueSize;
  thread_ = new std::thread(&fileWriter::writeSummary, this);
  nexttime =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  outfile.open(this->logdir, std::fstream::out |
      std::ios::trunc | std::ios::binary);
}

void fileWriter::writeSummary()
{
  // this is a thread that will continously write summary one by one into file
  while (true)
  {
    //  Break the loop if eveything is done
    if (!(close_ || q.size() != 0)) break;
    std::time_t timenow =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    if (timenow >= nexttime)
    {
      // wait for queue only if queue has any element or else it will
      // forever
      if (q.size() == 0) continue;
      mlboard::Event event = q.pop();
      std::string buf;
      event.SerializeToString(&buf);
      auto buf_len = static_cast<uint64_t>(buf.size());
      uint32_t len_crc =
          masked_crc32c((char *)&buf_len, sizeof(uint64_t));
      uint32_t data_crc = masked_crc32c(buf.c_str(), buf.size());

      outfile.write((char *)&buf_len, sizeof(uint64_t));
      outfile.write((char *)&len_crc, sizeof(uint32_t));
      outfile.write(buf.c_str(), buf.size());
      outfile.write((char *)&data_crc, sizeof(uint32_t));
      outfile.flush();
      nexttime =
          std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()
          + std::chrono::milliseconds(flushsec));
    }
  }
}

void fileWriter::createEvent(size_t step, mlboard::Summary *summary)
{
    Event event;
    double wall_time = time(nullptr);
    event.set_wall_time(wall_time);
    event.set_step(step);
    event.set_allocated_summary(summary);
    q.push(event);
}

void fileWriter::flush()
{
  // Flush everything succefully and close the thread.
  thread_->join();
}

void fileWriter::close()
{
  close_ = false;
  flush();
}

} // namespace mlboard

#endif
