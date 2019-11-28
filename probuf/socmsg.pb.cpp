// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: socmsg.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "socmsg.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace bd_soc_protobuf {

namespace {

const ::google::protobuf::Descriptor* Message_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  Message_reflection_ = NULL;
const ::google::protobuf::EnumDescriptor* Message_Mode_descriptor_ = NULL;
const ::google::protobuf::EnumDescriptor* Message_ServiceNo_descriptor_ = NULL;
const ::google::protobuf::EnumDescriptor* Message_enptypeNo_descriptor_ = NULL;
const ::google::protobuf::EnumDescriptor* Message_comtypeNo_descriptor_ = NULL;

}  // namespace


void protobuf_AssignDesc_socmsg_2eproto() {
  protobuf_AddDesc_socmsg_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "socmsg.proto");
  GOOGLE_CHECK(file != NULL);
  Message_descriptor_ = file->message_type(0);
  static const int Message_offsets_[10] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Message, serial_no_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Message, version_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Message, service_no_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Message, time_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Message, mode_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Message, code_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Message, enptype_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Message, comtype_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Message, desc_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Message, content_),
  };
  Message_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      Message_descriptor_,
      Message::default_instance_,
      Message_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Message, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Message, _unknown_fields_),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Message, _extensions_),
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(Message));
  Message_Mode_descriptor_ = Message_descriptor_->enum_type(0);
  Message_ServiceNo_descriptor_ = Message_descriptor_->enum_type(1);
  Message_enptypeNo_descriptor_ = Message_descriptor_->enum_type(2);
  Message_comtypeNo_descriptor_ = Message_descriptor_->enum_type(3);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_socmsg_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    Message_descriptor_, &Message::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_socmsg_2eproto() {
  delete Message::default_instance_;
  delete Message_reflection_;
}

void protobuf_AddDesc_socmsg_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\014socmsg.proto\022\017bd_soc_protobuf\"\367\006\n\007Mess"
    "age\022\021\n\tserial_no\030\001 \002(\004\022\031\n\007version\030\002 \001(\r:"
    "\01016777216\022\?\n\nservice_no\030\003 \001(\0162\".bd_soc_p"
    "rotobuf.Message.ServiceNo:\007SESSION\022\014\n\004ti"
    "me\030\004 \002(\004\0224\n\004mode\030\005 \001(\0162\035.bd_soc_protobuf"
    ".Message.Mode:\007REQUEST\022\017\n\004code\030\006 \001(\r:\0010\022"
    "\?\n\007enptype\030\007 \001(\0162\".bd_soc_protobuf.Messa"
    "ge.enptypeNo:\nNO_ENPTYPE\022@\n\007comtype\030\010 \001("
    "\0162\".bd_soc_protobuf.Message.comtypeNo:\013N"
    "O_COMPRESS\022\014\n\004desc\030\t \001(\t\022\017\n\007content\030\n \001("
    "\014\">\n\004Mode\022\013\n\007REQUEST\020\000\022\014\n\010RESPONSE\020\001\022\014\n\010"
    "REQUESTS\020\002\022\r\n\tRESPONSES\020\003\"\206\002\n\tServiceNo\022"
    "\016\n\010REGISTER\020\240\215\006\022\r\n\007SESSION\020\241\215\006\022\022\n\014START_"
    "PLUGIN\020\242\215\006\022\021\n\013STOP_PLUGIN\020\243\215\006\022\021\n\013START_A"
    "GENT\020\244\215\006\022\020\n\nSTOP_AGENT\020\245\215\006\022\022\n\014MONITOR_HO"
    "ST\020\246\215\006\022\022\n\014MANAGE_AGENT\020\247\215\006\022\020\n\nSTART_TASK"
    "\020\250\215\006\022\t\n\003LOG\020\300\232\014\022\021\n\013PERFORMANCE\020\301\232\014\022\r\n\007SE"
    "RVICE\020\302\232\014\022\n\n\004FLOW\020\303\232\014\022\r\n\007NETFLOW\020\304\232\014\022\014\n\006"
    "SYSTEM\020\277\204=\"l\n\tenptypeNo\022\017\n\nNO_ENPTYPE\020\240\037"
    "\022\016\n\tAES256ECB\020\241\037\022\016\n\tAES256CBC\020\242\037\022\016\n\tAES2"
    "56CFB\020\243\037\022\016\n\tAES256CTR\020\244\037\022\016\n\tAES256OFB\020\245\037"
    "\"E\n\tcomtypeNo\022\020\n\013NO_COMPRESS\020\210\'\022\022\n\rCOMPR"
    "ESS_ZLIB\020\211\'\022\022\n\rCOMPRESS_GZIP\020\212\'*\010\010\013\020\200\200\200\200"
    "\002:-\n\013register_sn\022\030.bd_soc_protobuf.Messa"
    "ge\030\013 \001(\004", 968);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "socmsg.proto", &protobuf_RegisterTypes);
  Message::default_instance_ = new Message();
  ::google::protobuf::internal::ExtensionSet::RegisterExtension(
    &::bd_soc_protobuf::Message::default_instance(),
    11, 4, false, false);
  Message::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_socmsg_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_socmsg_2eproto {
  StaticDescriptorInitializer_socmsg_2eproto() {
    protobuf_AddDesc_socmsg_2eproto();
  }
} static_descriptor_initializer_socmsg_2eproto_;

// ===================================================================

const ::google::protobuf::EnumDescriptor* Message_Mode_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return Message_Mode_descriptor_;
}
bool Message_Mode_IsValid(int value) {
  switch(value) {
    case 0:
    case 1:
    case 2:
    case 3:
      return true;
    default:
      return false;
  }
}

#ifndef _MSC_VER
const Message_Mode Message::REQUEST;
const Message_Mode Message::RESPONSE;
const Message_Mode Message::REQUESTS;
const Message_Mode Message::RESPONSES;
const Message_Mode Message::Mode_MIN;
const Message_Mode Message::Mode_MAX;
const int Message::Mode_ARRAYSIZE;
#endif  // _MSC_VER
const ::google::protobuf::EnumDescriptor* Message_ServiceNo_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return Message_ServiceNo_descriptor_;
}
bool Message_ServiceNo_IsValid(int value) {
  switch(value) {
    case 100000:
    case 100001:
    case 100002:
    case 100003:
    case 100004:
    case 100005:
    case 100006:
    case 100007:
    case 100008:
    case 200000:
    case 200001:
    case 200002:
    case 200003:
    case 200004:
    case 999999:
      return true;
    default:
      return false;
  }
}

#ifndef _MSC_VER
const Message_ServiceNo Message::REGISTER;
const Message_ServiceNo Message::SESSION;
const Message_ServiceNo Message::START_PLUGIN;
const Message_ServiceNo Message::STOP_PLUGIN;
const Message_ServiceNo Message::START_AGENT;
const Message_ServiceNo Message::STOP_AGENT;
const Message_ServiceNo Message::MONITOR_HOST;
const Message_ServiceNo Message::MANAGE_AGENT;
const Message_ServiceNo Message::START_TASK;
const Message_ServiceNo Message::LOG;
const Message_ServiceNo Message::PERFORMANCE;
const Message_ServiceNo Message::SERVICE;
const Message_ServiceNo Message::FLOW;
const Message_ServiceNo Message::NETFLOW;
const Message_ServiceNo Message::SYSTEM;
const Message_ServiceNo Message::ServiceNo_MIN;
const Message_ServiceNo Message::ServiceNo_MAX;
const int Message::ServiceNo_ARRAYSIZE;
#endif  // _MSC_VER
const ::google::protobuf::EnumDescriptor* Message_enptypeNo_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return Message_enptypeNo_descriptor_;
}
bool Message_enptypeNo_IsValid(int value) {
  switch(value) {
    case 4000:
    case 4001:
    case 4002:
    case 4003:
    case 4004:
    case 4005:
      return true;
    default:
      return false;
  }
}

#ifndef _MSC_VER
const Message_enptypeNo Message::NO_ENPTYPE;
const Message_enptypeNo Message::AES256ECB;
const Message_enptypeNo Message::AES256CBC;
const Message_enptypeNo Message::AES256CFB;
const Message_enptypeNo Message::AES256CTR;
const Message_enptypeNo Message::AES256OFB;
const Message_enptypeNo Message::enptypeNo_MIN;
const Message_enptypeNo Message::enptypeNo_MAX;
const int Message::enptypeNo_ARRAYSIZE;
#endif  // _MSC_VER
const ::google::protobuf::EnumDescriptor* Message_comtypeNo_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return Message_comtypeNo_descriptor_;
}
bool Message_comtypeNo_IsValid(int value) {
  switch(value) {
    case 5000:
    case 5001:
    case 5002:
      return true;
    default:
      return false;
  }
}

#ifndef _MSC_VER
const Message_comtypeNo Message::NO_COMPRESS;
const Message_comtypeNo Message::COMPRESS_ZLIB;
const Message_comtypeNo Message::COMPRESS_GZIP;
const Message_comtypeNo Message::comtypeNo_MIN;
const Message_comtypeNo Message::comtypeNo_MAX;
const int Message::comtypeNo_ARRAYSIZE;
#endif  // _MSC_VER
#ifndef _MSC_VER
const int Message::kSerialNoFieldNumber;
const int Message::kVersionFieldNumber;
const int Message::kServiceNoFieldNumber;
const int Message::kTimeFieldNumber;
const int Message::kModeFieldNumber;
const int Message::kCodeFieldNumber;
const int Message::kEnptypeFieldNumber;
const int Message::kComtypeFieldNumber;
const int Message::kDescFieldNumber;
const int Message::kContentFieldNumber;
#endif  // !_MSC_VER

Message::Message()
  : ::google::protobuf::Message() {
  SharedCtor();
  // @@protoc_insertion_point(constructor:bd_soc_protobuf.Message)
}

void Message::InitAsDefaultInstance() {
}

Message::Message(const Message& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:bd_soc_protobuf.Message)
}

void Message::SharedCtor() {
  ::google::protobuf::internal::GetEmptyString();
  _cached_size_ = 0;
  serial_no_ = GOOGLE_ULONGLONG(0);
  version_ = 16777216u;
  service_no_ = 100001;
  time_ = GOOGLE_ULONGLONG(0);
  mode_ = 0;
  code_ = 0u;
  enptype_ = 4000;
  comtype_ = 5000;
  desc_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  content_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

Message::~Message() {
  // @@protoc_insertion_point(destructor:bd_soc_protobuf.Message)
  SharedDtor();
}

void Message::SharedDtor() {
  if (desc_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete desc_;
  }
  if (content_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete content_;
  }
  if (this != default_instance_) {
  }
}

void Message::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* Message::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return Message_descriptor_;
}

const Message& Message::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_socmsg_2eproto();
  return *default_instance_;
}

Message* Message::default_instance_ = NULL;

Message* Message::New() const {
  return new Message;
}

void Message::Clear() {
  _extensions_.Clear();
#define OFFSET_OF_FIELD_(f) (reinterpret_cast<char*>(      \
  &reinterpret_cast<Message*>(16)->f) - \
   reinterpret_cast<char*>(16))

#define ZR_(first, last) do {                              \
    size_t f = OFFSET_OF_FIELD_(first);                    \
    size_t n = OFFSET_OF_FIELD_(last) - f + sizeof(last);  \
    ::memset(&first, 0, n);                                \
  } while (0)

  if (_has_bits_[0 / 32] & 255) {
    ZR_(time_, code_);
    serial_no_ = GOOGLE_ULONGLONG(0);
    version_ = 16777216u;
    service_no_ = 100001;
    enptype_ = 4000;
    comtype_ = 5000;
  }
  if (_has_bits_[8 / 32] & 768) {
    if (has_desc()) {
      if (desc_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
        desc_->clear();
      }
    }
    if (has_content()) {
      if (content_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
        content_->clear();
      }
    }
  }

#undef OFFSET_OF_FIELD_
#undef ZR_

  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool Message::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:bd_soc_protobuf.Message)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required uint64 serial_no = 1;
      case 1: {
        if (tag == 8) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint64, ::google::protobuf::internal::WireFormatLite::TYPE_UINT64>(
                 input, &serial_no_)));
          set_has_serial_no();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(16)) goto parse_version;
        break;
      }

      // optional uint32 version = 2 [default = 16777216];
      case 2: {
        if (tag == 16) {
         parse_version:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &version_)));
          set_has_version();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(24)) goto parse_service_no;
        break;
      }

      // optional .bd_soc_protobuf.Message.ServiceNo service_no = 3 [default = SESSION];
      case 3: {
        if (tag == 24) {
         parse_service_no:
          int value;
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::bd_soc_protobuf::Message_ServiceNo_IsValid(value)) {
            set_service_no(static_cast< ::bd_soc_protobuf::Message_ServiceNo >(value));
          } else {
            mutable_unknown_fields()->AddVarint(3, value);
          }
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(32)) goto parse_time;
        break;
      }

      // required uint64 time = 4;
      case 4: {
        if (tag == 32) {
         parse_time:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint64, ::google::protobuf::internal::WireFormatLite::TYPE_UINT64>(
                 input, &time_)));
          set_has_time();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(40)) goto parse_mode;
        break;
      }

      // optional .bd_soc_protobuf.Message.Mode mode = 5 [default = REQUEST];
      case 5: {
        if (tag == 40) {
         parse_mode:
          int value;
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::bd_soc_protobuf::Message_Mode_IsValid(value)) {
            set_mode(static_cast< ::bd_soc_protobuf::Message_Mode >(value));
          } else {
            mutable_unknown_fields()->AddVarint(5, value);
          }
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(48)) goto parse_code;
        break;
      }

      // optional uint32 code = 6 [default = 0];
      case 6: {
        if (tag == 48) {
         parse_code:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &code_)));
          set_has_code();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(56)) goto parse_enptype;
        break;
      }

      // optional .bd_soc_protobuf.Message.enptypeNo enptype = 7 [default = NO_ENPTYPE];
      case 7: {
        if (tag == 56) {
         parse_enptype:
          int value;
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::bd_soc_protobuf::Message_enptypeNo_IsValid(value)) {
            set_enptype(static_cast< ::bd_soc_protobuf::Message_enptypeNo >(value));
          } else {
            mutable_unknown_fields()->AddVarint(7, value);
          }
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(64)) goto parse_comtype;
        break;
      }

      // optional .bd_soc_protobuf.Message.comtypeNo comtype = 8 [default = NO_COMPRESS];
      case 8: {
        if (tag == 64) {
         parse_comtype:
          int value;
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   int, ::google::protobuf::internal::WireFormatLite::TYPE_ENUM>(
                 input, &value)));
          if (::bd_soc_protobuf::Message_comtypeNo_IsValid(value)) {
            set_comtype(static_cast< ::bd_soc_protobuf::Message_comtypeNo >(value));
          } else {
            mutable_unknown_fields()->AddVarint(8, value);
          }
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(74)) goto parse_desc;
        break;
      }

      // optional string desc = 9;
      case 9: {
        if (tag == 74) {
         parse_desc:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_desc()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->desc().data(), this->desc().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "desc");
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(82)) goto parse_content;
        break;
      }

      // optional bytes content = 10;
      case 10: {
        if (tag == 82) {
         parse_content:
          DO_(::google::protobuf::internal::WireFormatLite::ReadBytes(
                input, this->mutable_content()));
        } else {
          goto handle_unusual;
        }
        if (input->ExpectAtEnd()) goto success;
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        if ((88u <= tag)) {
          DO_(_extensions_.ParseField(tag, input, default_instance_,
                                      mutable_unknown_fields()));
          continue;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:bd_soc_protobuf.Message)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:bd_soc_protobuf.Message)
  return false;
#undef DO_
}

void Message::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:bd_soc_protobuf.Message)
  // required uint64 serial_no = 1;
  if (has_serial_no()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt64(1, this->serial_no(), output);
  }

  // optional uint32 version = 2 [default = 16777216];
  if (has_version()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(2, this->version(), output);
  }

  // optional .bd_soc_protobuf.Message.ServiceNo service_no = 3 [default = SESSION];
  if (has_service_no()) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      3, this->service_no(), output);
  }

  // required uint64 time = 4;
  if (has_time()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt64(4, this->time(), output);
  }

  // optional .bd_soc_protobuf.Message.Mode mode = 5 [default = REQUEST];
  if (has_mode()) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      5, this->mode(), output);
  }

  // optional uint32 code = 6 [default = 0];
  if (has_code()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(6, this->code(), output);
  }

  // optional .bd_soc_protobuf.Message.enptypeNo enptype = 7 [default = NO_ENPTYPE];
  if (has_enptype()) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      7, this->enptype(), output);
  }

  // optional .bd_soc_protobuf.Message.comtypeNo comtype = 8 [default = NO_COMPRESS];
  if (has_comtype()) {
    ::google::protobuf::internal::WireFormatLite::WriteEnum(
      8, this->comtype(), output);
  }

  // optional string desc = 9;
  if (has_desc()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->desc().data(), this->desc().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "desc");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      9, this->desc(), output);
  }

  // optional bytes content = 10;
  if (has_content()) {
    ::google::protobuf::internal::WireFormatLite::WriteBytesMaybeAliased(
      10, this->content(), output);
  }

  // Extension range [11, 536870912)
  _extensions_.SerializeWithCachedSizes(
      11, 536870912, output);

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:bd_soc_protobuf.Message)
}

::google::protobuf::uint8* Message::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:bd_soc_protobuf.Message)
  // required uint64 serial_no = 1;
  if (has_serial_no()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt64ToArray(1, this->serial_no(), target);
  }

  // optional uint32 version = 2 [default = 16777216];
  if (has_version()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(2, this->version(), target);
  }

  // optional .bd_soc_protobuf.Message.ServiceNo service_no = 3 [default = SESSION];
  if (has_service_no()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteEnumToArray(
      3, this->service_no(), target);
  }

  // required uint64 time = 4;
  if (has_time()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt64ToArray(4, this->time(), target);
  }

  // optional .bd_soc_protobuf.Message.Mode mode = 5 [default = REQUEST];
  if (has_mode()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteEnumToArray(
      5, this->mode(), target);
  }

  // optional uint32 code = 6 [default = 0];
  if (has_code()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(6, this->code(), target);
  }

  // optional .bd_soc_protobuf.Message.enptypeNo enptype = 7 [default = NO_ENPTYPE];
  if (has_enptype()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteEnumToArray(
      7, this->enptype(), target);
  }

  // optional .bd_soc_protobuf.Message.comtypeNo comtype = 8 [default = NO_COMPRESS];
  if (has_comtype()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteEnumToArray(
      8, this->comtype(), target);
  }

  // optional string desc = 9;
  if (has_desc()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->desc().data(), this->desc().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "desc");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        9, this->desc(), target);
  }

  // optional bytes content = 10;
  if (has_content()) {
    target =
      ::google::protobuf::internal::WireFormatLite::WriteBytesToArray(
        10, this->content(), target);
  }

  // Extension range [11, 536870912)
  target = _extensions_.SerializeWithCachedSizesToArray(
      11, 536870912, target);

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:bd_soc_protobuf.Message)
  return target;
}

int Message::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required uint64 serial_no = 1;
    if (has_serial_no()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt64Size(
          this->serial_no());
    }

    // optional uint32 version = 2 [default = 16777216];
    if (has_version()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->version());
    }

    // optional .bd_soc_protobuf.Message.ServiceNo service_no = 3 [default = SESSION];
    if (has_service_no()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::EnumSize(this->service_no());
    }

    // required uint64 time = 4;
    if (has_time()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt64Size(
          this->time());
    }

    // optional .bd_soc_protobuf.Message.Mode mode = 5 [default = REQUEST];
    if (has_mode()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::EnumSize(this->mode());
    }

    // optional uint32 code = 6 [default = 0];
    if (has_code()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt32Size(
          this->code());
    }

    // optional .bd_soc_protobuf.Message.enptypeNo enptype = 7 [default = NO_ENPTYPE];
    if (has_enptype()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::EnumSize(this->enptype());
    }

    // optional .bd_soc_protobuf.Message.comtypeNo comtype = 8 [default = NO_COMPRESS];
    if (has_comtype()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::EnumSize(this->comtype());
    }

  }
  if (_has_bits_[8 / 32] & (0xffu << (8 % 32))) {
    // optional string desc = 9;
    if (has_desc()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->desc());
    }

    // optional bytes content = 10;
    if (has_content()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::BytesSize(
          this->content());
    }

  }
  total_size += _extensions_.ByteSize();

  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void Message::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const Message* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const Message*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void Message::MergeFrom(const Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_serial_no()) {
      set_serial_no(from.serial_no());
    }
    if (from.has_version()) {
      set_version(from.version());
    }
    if (from.has_service_no()) {
      set_service_no(from.service_no());
    }
    if (from.has_time()) {
      set_time(from.time());
    }
    if (from.has_mode()) {
      set_mode(from.mode());
    }
    if (from.has_code()) {
      set_code(from.code());
    }
    if (from.has_enptype()) {
      set_enptype(from.enptype());
    }
    if (from.has_comtype()) {
      set_comtype(from.comtype());
    }
  }
  if (from._has_bits_[8 / 32] & (0xffu << (8 % 32))) {
    if (from.has_desc()) {
      set_desc(from.desc());
    }
    if (from.has_content()) {
      set_content(from.content());
    }
  }
  _extensions_.MergeFrom(from._extensions_);
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void Message::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void Message::CopyFrom(const Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Message::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000009) != 0x00000009) return false;


  if (!_extensions_.IsInitialized()) return false;  return true;
}

void Message::Swap(Message* other) {
  if (other != this) {
    std::swap(serial_no_, other->serial_no_);
    std::swap(version_, other->version_);
    std::swap(service_no_, other->service_no_);
    std::swap(time_, other->time_);
    std::swap(mode_, other->mode_);
    std::swap(code_, other->code_);
    std::swap(enptype_, other->enptype_);
    std::swap(comtype_, other->comtype_);
    std::swap(desc_, other->desc_);
    std::swap(content_, other->content_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
    _extensions_.Swap(&other->_extensions_);
  }
}

::google::protobuf::Metadata Message::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = Message_descriptor_;
  metadata.reflection = Message_reflection_;
  return metadata;
}

::google::protobuf::internal::ExtensionIdentifier< ::bd_soc_protobuf::Message,
    ::google::protobuf::internal::PrimitiveTypeTraits< ::google::protobuf::uint64 >, 4, false >
  register_sn(kRegisterSnFieldNumber, GOOGLE_ULONGLONG(0));

// @@protoc_insertion_point(namespace_scope)

}  // namespace bd_soc_protobuf

// @@protoc_insertion_point(global_scope)