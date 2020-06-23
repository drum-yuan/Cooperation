#pragma once

// The MIT License (MIT)
//
// Copyright (c) 2014 Siyuan Ren (netheril96@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <autojsoncxx/autojsoncxx.hpp>

// The comments are reserved for replacement
// such syntax is chosen so that the template file looks like valid C++

namespace MCUProtocol { struct VideoAck_C2S {
 unsigned int sequence;

explicit VideoAck_C2S():sequence() {  }


 
}; }


namespace autojsoncxx {

template <>
class SAXEventHandler< ::MCUProtocol::VideoAck_C2S > {
private:
    utility::scoped_ptr<error::ErrorBase> the_error;
    int state;
    int depth;

    SAXEventHandler< unsigned int > handler_0;

    bool check_depth(const char* type)
    {
        if (depth <= 0) {
            the_error.reset(new error::TypeMismatchError("object", type));
            return false;
        }
        return true;
    }

    const char* current_member_name() const
    {
        switch (state) {
            case 0:
    return "sequence";
        default:
            break;
        }
        return "<UNKNOWN>";
    }

    bool checked_event_forwarding(bool success)
    {
        if (!success)
            the_error.reset(new error::ObjectMemberError(current_member_name()));
        return success;
    }

    void set_missing_required(const char* name)
    {
        if (the_error.empty() || the_error->type() != error::MISSING_REQUIRED)
            the_error.reset(new error::RequiredFieldMissingError());

        std::vector<std::string>& missing =
            static_cast<error::RequiredFieldMissingError*>(the_error.get())->missing_members();

        missing.push_back(name);
    }

    void reset_flags()
    {
        
    }

public:
    explicit SAXEventHandler( ::MCUProtocol::VideoAck_C2S * obj)
        : state(-1)
        , depth(0)
        , handler_0(&obj->sequence)
    {
        reset_flags();
    }

    bool Null()
    {
        if (!check_depth("null"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Null());

        default:
            break;
        }
        return true;
    }

    bool Bool(bool b)
    {
        if (!check_depth("bool"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Bool(b));

        default:
            break;
        }
        return true;
    }

    bool Int(int i)
    {
        if (!check_depth("int"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Int(i));

        default:
            break;
        }
        return true;
    }

    bool Uint(unsigned i)
    {
        if (!check_depth("unsigned"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Uint(i));

        default:
            break;
        }
        return true;
    }

    bool Int64(utility::int64_t i)
    {
        if (!check_depth("int64_t"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Int64(i));

        default:
            break;
        }
        return true;
    }

    bool Uint64(utility::uint64_t i)
    {
        if (!check_depth("uint64_t"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Uint64(i));

        default:
            break;
        }
        return true;
    }

    bool Double(double d)
    {
        if (!check_depth("double"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Double(d));

        default:
            break;
        }
        return true;
    }

    bool String(const char* str, SizeType length, bool copy)
    {
        if (!check_depth("string"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.String(str, length, copy));

        default:
            break;
        }
        return true;
    }

    bool Key(const char* str, SizeType length, bool copy)
    {
        if (!check_depth("object"))
            return false;

        if (depth == 1) {
            if (0) {
            }
            else if (utility::string_equal(str, length, "\x73\x65\x71\x75\x65\x6e\x63\x65", 8))
                         { state=0;  }
            else {
                state = -1;
                return true;
            }

        } else {
            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.Key(str, length, copy));

            default:
                break;
            }
        }
        return true;
    }

    bool StartArray()
    {
        if (!check_depth("array"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.StartArray());

        default:
            break;
        }
        return true;
    }

    bool EndArray(SizeType length)
    {
        if (!check_depth("array"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.EndArray(length));

        default:
            break;
        }
        return true;
    }

    bool StartObject()
    {
        ++depth;
        if (depth > 1) {

            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.StartObject());

            default:
                break;
            }
        }
        return true;
    }

    bool EndObject(SizeType length)
    {
        --depth;
        if (depth > 0) {

            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.EndObject(length));

            default:
                break;
            }
        } else {
            
        }
        return the_error.empty();
    }

    bool HasError() const
    {
        return !this->the_error.empty();
    }

    bool ReapError(error::ErrorStack& errs)
    {
        if (this->the_error.empty())
            return false;

        errs.push(this->the_error.release());

        switch (state) {

        case 0:
     handler_0.ReapError(errs); break;

        default:
            break;
        }

        return true;
    }

    void PrepareForReuse()
    {
        depth = 0;
        state = -1;
        the_error.reset();
        reset_flags();
        handler_0.PrepareForReuse();

    }
};

template < class Writer8a4db7b63c2539b79ca116f009d6b5374f5d083552281eb63a142644f03fff4d >
struct Serializer< Writer8a4db7b63c2539b79ca116f009d6b5374f5d083552281eb63a142644f03fff4d, ::MCUProtocol::VideoAck_C2S > {

    void operator()( Writer8a4db7b63c2539b79ca116f009d6b5374f5d083552281eb63a142644f03fff4d& w, const ::MCUProtocol::VideoAck_C2S& value) const
    {
        w.StartObject();

        w.Key("\x73\x65\x71\x75\x65\x6e\x63\x65", 8, false); Serializer< Writer8a4db7b63c2539b79ca116f009d6b5374f5d083552281eb63a142644f03fff4d, unsigned int >()(w, value.sequence);

        w.EndObject(1);
    }

};
}


// The MIT License (MIT)
//
// Copyright (c) 2014 Siyuan Ren (netheril96@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <autojsoncxx/autojsoncxx.hpp>

// The comments are reserved for replacement
// such syntax is chosen so that the template file looks like valid C++

namespace MCUProtocol { struct RequestKeyFrame {
 bool resetMcuBuf;
bool resetSequence;

explicit RequestKeyFrame():resetMcuBuf(), resetSequence() {  }


 
}; }


namespace autojsoncxx {

template <>
class SAXEventHandler< ::MCUProtocol::RequestKeyFrame > {
private:
    utility::scoped_ptr<error::ErrorBase> the_error;
    int state;
    int depth;

    SAXEventHandler< bool > handler_0;
SAXEventHandler< bool > handler_1;

    bool check_depth(const char* type)
    {
        if (depth <= 0) {
            the_error.reset(new error::TypeMismatchError("object", type));
            return false;
        }
        return true;
    }

    const char* current_member_name() const
    {
        switch (state) {
            case 0:
    return "resetMcuBuf";
case 1:
    return "resetSequence";
        default:
            break;
        }
        return "<UNKNOWN>";
    }

    bool checked_event_forwarding(bool success)
    {
        if (!success)
            the_error.reset(new error::ObjectMemberError(current_member_name()));
        return success;
    }

    void set_missing_required(const char* name)
    {
        if (the_error.empty() || the_error->type() != error::MISSING_REQUIRED)
            the_error.reset(new error::RequiredFieldMissingError());

        std::vector<std::string>& missing =
            static_cast<error::RequiredFieldMissingError*>(the_error.get())->missing_members();

        missing.push_back(name);
    }

    void reset_flags()
    {
        

    }

public:
    explicit SAXEventHandler( ::MCUProtocol::RequestKeyFrame * obj)
        : state(-1)
        , depth(0)
        , handler_0(&obj->resetMcuBuf)
, handler_1(&obj->resetSequence)
    {
        reset_flags();
    }

    bool Null()
    {
        if (!check_depth("null"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Null());

case 1:
    return checked_event_forwarding(handler_1.Null());

        default:
            break;
        }
        return true;
    }

    bool Bool(bool b)
    {
        if (!check_depth("bool"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Bool(b));

case 1:
    return checked_event_forwarding(handler_1.Bool(b));

        default:
            break;
        }
        return true;
    }

    bool Int(int i)
    {
        if (!check_depth("int"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Int(i));

case 1:
    return checked_event_forwarding(handler_1.Int(i));

        default:
            break;
        }
        return true;
    }

    bool Uint(unsigned i)
    {
        if (!check_depth("unsigned"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Uint(i));

case 1:
    return checked_event_forwarding(handler_1.Uint(i));

        default:
            break;
        }
        return true;
    }

    bool Int64(utility::int64_t i)
    {
        if (!check_depth("int64_t"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Int64(i));

case 1:
    return checked_event_forwarding(handler_1.Int64(i));

        default:
            break;
        }
        return true;
    }

    bool Uint64(utility::uint64_t i)
    {
        if (!check_depth("uint64_t"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Uint64(i));

case 1:
    return checked_event_forwarding(handler_1.Uint64(i));

        default:
            break;
        }
        return true;
    }

    bool Double(double d)
    {
        if (!check_depth("double"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Double(d));

case 1:
    return checked_event_forwarding(handler_1.Double(d));

        default:
            break;
        }
        return true;
    }

    bool String(const char* str, SizeType length, bool copy)
    {
        if (!check_depth("string"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.String(str, length, copy));

case 1:
    return checked_event_forwarding(handler_1.String(str, length, copy));

        default:
            break;
        }
        return true;
    }

    bool Key(const char* str, SizeType length, bool copy)
    {
        if (!check_depth("object"))
            return false;

        if (depth == 1) {
            if (0) {
            }
            else if (utility::string_equal(str, length, "\x72\x65\x73\x65\x74\x4d\x63\x75\x42\x75\x66", 11))
                         { state=0;  }
else if (utility::string_equal(str, length, "\x72\x65\x73\x65\x74\x53\x65\x71\x75\x65\x6e\x63\x65", 13))
                         { state=1;  }
            else {
                state = -1;
                return true;
            }

        } else {
            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.Key(str, length, copy));

case 1:
    return checked_event_forwarding(handler_1.Key(str, length, copy));

            default:
                break;
            }
        }
        return true;
    }

    bool StartArray()
    {
        if (!check_depth("array"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.StartArray());

case 1:
    return checked_event_forwarding(handler_1.StartArray());

        default:
            break;
        }
        return true;
    }

    bool EndArray(SizeType length)
    {
        if (!check_depth("array"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.EndArray(length));

case 1:
    return checked_event_forwarding(handler_1.EndArray(length));

        default:
            break;
        }
        return true;
    }

    bool StartObject()
    {
        ++depth;
        if (depth > 1) {

            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.StartObject());

case 1:
    return checked_event_forwarding(handler_1.StartObject());

            default:
                break;
            }
        }
        return true;
    }

    bool EndObject(SizeType length)
    {
        --depth;
        if (depth > 0) {

            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.EndObject(length));

case 1:
    return checked_event_forwarding(handler_1.EndObject(length));

            default:
                break;
            }
        } else {
            
        }
        return the_error.empty();
    }

    bool HasError() const
    {
        return !this->the_error.empty();
    }

    bool ReapError(error::ErrorStack& errs)
    {
        if (this->the_error.empty())
            return false;

        errs.push(this->the_error.release());

        switch (state) {

        case 0:
     handler_0.ReapError(errs); break;
case 1:
     handler_1.ReapError(errs); break;

        default:
            break;
        }

        return true;
    }

    void PrepareForReuse()
    {
        depth = 0;
        state = -1;
        the_error.reset();
        reset_flags();
        handler_0.PrepareForReuse();
handler_1.PrepareForReuse();

    }
};

template < class Writer293ec667a0e005a97d686398d22ff3ce4d4ca4acca7e3e2edc2388892b878624 >
struct Serializer< Writer293ec667a0e005a97d686398d22ff3ce4d4ca4acca7e3e2edc2388892b878624, ::MCUProtocol::RequestKeyFrame > {

    void operator()( Writer293ec667a0e005a97d686398d22ff3ce4d4ca4acca7e3e2edc2388892b878624& w, const ::MCUProtocol::RequestKeyFrame& value) const
    {
        w.StartObject();

        w.Key("\x72\x65\x73\x65\x74\x4d\x63\x75\x42\x75\x66", 11, false); Serializer< Writer293ec667a0e005a97d686398d22ff3ce4d4ca4acca7e3e2edc2388892b878624, bool >()(w, value.resetMcuBuf);
w.Key("\x72\x65\x73\x65\x74\x53\x65\x71\x75\x65\x6e\x63\x65", 13, false); Serializer< Writer293ec667a0e005a97d686398d22ff3ce4d4ca4acca7e3e2edc2388892b878624, bool >()(w, value.resetSequence);

        w.EndObject(2);
    }

};
}


// The MIT License (MIT)
//
// Copyright (c) 2014 Siyuan Ren (netheril96@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <autojsoncxx/autojsoncxx.hpp>

// The comments are reserved for replacement
// such syntax is chosen so that the template file looks like valid C++

namespace MCUProtocol { struct MouseInput_C2S {
 unsigned int posX;
unsigned int posY;
unsigned int buttonMask;

explicit MouseInput_C2S():posX(), posY(), buttonMask() {  }


 
}; }


namespace autojsoncxx {

template <>
class SAXEventHandler< ::MCUProtocol::MouseInput_C2S > {
private:
    utility::scoped_ptr<error::ErrorBase> the_error;
    int state;
    int depth;

    SAXEventHandler< unsigned int > handler_0;
SAXEventHandler< unsigned int > handler_1;
SAXEventHandler< unsigned int > handler_2;bool has_posX;
bool has_posY;
bool has_buttonMask;

    bool check_depth(const char* type)
    {
        if (depth <= 0) {
            the_error.reset(new error::TypeMismatchError("object", type));
            return false;
        }
        return true;
    }

    const char* current_member_name() const
    {
        switch (state) {
            case 0:
    return "posX";
case 1:
    return "posY";
case 2:
    return "buttonMask";
        default:
            break;
        }
        return "<UNKNOWN>";
    }

    bool checked_event_forwarding(bool success)
    {
        if (!success)
            the_error.reset(new error::ObjectMemberError(current_member_name()));
        return success;
    }

    void set_missing_required(const char* name)
    {
        if (the_error.empty() || the_error->type() != error::MISSING_REQUIRED)
            the_error.reset(new error::RequiredFieldMissingError());

        std::vector<std::string>& missing =
            static_cast<error::RequiredFieldMissingError*>(the_error.get())->missing_members();

        missing.push_back(name);
    }

    void reset_flags()
    {
        has_posX = false;
has_posY = false;
has_buttonMask = false;
    }

public:
    explicit SAXEventHandler( ::MCUProtocol::MouseInput_C2S * obj)
        : state(-1)
        , depth(0)
        , handler_0(&obj->posX)
, handler_1(&obj->posY)
, handler_2(&obj->buttonMask)
    {
        reset_flags();
    }

    bool Null()
    {
        if (!check_depth("null"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Null());

case 1:
    return checked_event_forwarding(handler_1.Null());

case 2:
    return checked_event_forwarding(handler_2.Null());

        default:
            break;
        }
        return true;
    }

    bool Bool(bool b)
    {
        if (!check_depth("bool"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Bool(b));

case 1:
    return checked_event_forwarding(handler_1.Bool(b));

case 2:
    return checked_event_forwarding(handler_2.Bool(b));

        default:
            break;
        }
        return true;
    }

    bool Int(int i)
    {
        if (!check_depth("int"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Int(i));

case 1:
    return checked_event_forwarding(handler_1.Int(i));

case 2:
    return checked_event_forwarding(handler_2.Int(i));

        default:
            break;
        }
        return true;
    }

    bool Uint(unsigned i)
    {
        if (!check_depth("unsigned"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Uint(i));

case 1:
    return checked_event_forwarding(handler_1.Uint(i));

case 2:
    return checked_event_forwarding(handler_2.Uint(i));

        default:
            break;
        }
        return true;
    }

    bool Int64(utility::int64_t i)
    {
        if (!check_depth("int64_t"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Int64(i));

case 1:
    return checked_event_forwarding(handler_1.Int64(i));

case 2:
    return checked_event_forwarding(handler_2.Int64(i));

        default:
            break;
        }
        return true;
    }

    bool Uint64(utility::uint64_t i)
    {
        if (!check_depth("uint64_t"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Uint64(i));

case 1:
    return checked_event_forwarding(handler_1.Uint64(i));

case 2:
    return checked_event_forwarding(handler_2.Uint64(i));

        default:
            break;
        }
        return true;
    }

    bool Double(double d)
    {
        if (!check_depth("double"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Double(d));

case 1:
    return checked_event_forwarding(handler_1.Double(d));

case 2:
    return checked_event_forwarding(handler_2.Double(d));

        default:
            break;
        }
        return true;
    }

    bool String(const char* str, SizeType length, bool copy)
    {
        if (!check_depth("string"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.String(str, length, copy));

case 1:
    return checked_event_forwarding(handler_1.String(str, length, copy));

case 2:
    return checked_event_forwarding(handler_2.String(str, length, copy));

        default:
            break;
        }
        return true;
    }

    bool Key(const char* str, SizeType length, bool copy)
    {
        if (!check_depth("object"))
            return false;

        if (depth == 1) {
            if (0) {
            }
            else if (utility::string_equal(str, length, "\x70\x6f\x73\x58", 4))
                         { state=0; has_posX = true; }
else if (utility::string_equal(str, length, "\x70\x6f\x73\x59", 4))
                         { state=1; has_posY = true; }
else if (utility::string_equal(str, length, "\x62\x75\x74\x74\x6f\x6e\x4d\x61\x73\x6b", 10))
                         { state=2; has_buttonMask = true; }
            else {
                state = -1;
                return true;
            }

        } else {
            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.Key(str, length, copy));

case 1:
    return checked_event_forwarding(handler_1.Key(str, length, copy));

case 2:
    return checked_event_forwarding(handler_2.Key(str, length, copy));

            default:
                break;
            }
        }
        return true;
    }

    bool StartArray()
    {
        if (!check_depth("array"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.StartArray());

case 1:
    return checked_event_forwarding(handler_1.StartArray());

case 2:
    return checked_event_forwarding(handler_2.StartArray());

        default:
            break;
        }
        return true;
    }

    bool EndArray(SizeType length)
    {
        if (!check_depth("array"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.EndArray(length));

case 1:
    return checked_event_forwarding(handler_1.EndArray(length));

case 2:
    return checked_event_forwarding(handler_2.EndArray(length));

        default:
            break;
        }
        return true;
    }

    bool StartObject()
    {
        ++depth;
        if (depth > 1) {

            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.StartObject());

case 1:
    return checked_event_forwarding(handler_1.StartObject());

case 2:
    return checked_event_forwarding(handler_2.StartObject());

            default:
                break;
            }
        }
        return true;
    }

    bool EndObject(SizeType length)
    {
        --depth;
        if (depth > 0) {

            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.EndObject(length));

case 1:
    return checked_event_forwarding(handler_1.EndObject(length));

case 2:
    return checked_event_forwarding(handler_2.EndObject(length));

            default:
                break;
            }
        } else {
            if (!has_posX) set_missing_required("posX");
if (!has_posY) set_missing_required("posY");
if (!has_buttonMask) set_missing_required("buttonMask");
        }
        return the_error.empty();
    }

    bool HasError() const
    {
        return !this->the_error.empty();
    }

    bool ReapError(error::ErrorStack& errs)
    {
        if (this->the_error.empty())
            return false;

        errs.push(this->the_error.release());

        switch (state) {

        case 0:
     handler_0.ReapError(errs); break;
case 1:
     handler_1.ReapError(errs); break;
case 2:
     handler_2.ReapError(errs); break;

        default:
            break;
        }

        return true;
    }

    void PrepareForReuse()
    {
        depth = 0;
        state = -1;
        the_error.reset();
        reset_flags();
        handler_0.PrepareForReuse();
handler_1.PrepareForReuse();
handler_2.PrepareForReuse();

    }
};

template < class Writer288757398cd2a69ce1185dc37c9d0af88b46f08d2e04d695908880956654ef94 >
struct Serializer< Writer288757398cd2a69ce1185dc37c9d0af88b46f08d2e04d695908880956654ef94, ::MCUProtocol::MouseInput_C2S > {

    void operator()( Writer288757398cd2a69ce1185dc37c9d0af88b46f08d2e04d695908880956654ef94& w, const ::MCUProtocol::MouseInput_C2S& value) const
    {
        w.StartObject();

        w.Key("\x70\x6f\x73\x58", 4, false); Serializer< Writer288757398cd2a69ce1185dc37c9d0af88b46f08d2e04d695908880956654ef94, unsigned int >()(w, value.posX);
w.Key("\x70\x6f\x73\x59", 4, false); Serializer< Writer288757398cd2a69ce1185dc37c9d0af88b46f08d2e04d695908880956654ef94, unsigned int >()(w, value.posY);
w.Key("\x62\x75\x74\x74\x6f\x6e\x4d\x61\x73\x6b", 10, false); Serializer< Writer288757398cd2a69ce1185dc37c9d0af88b46f08d2e04d695908880956654ef94, unsigned int >()(w, value.buttonMask);

        w.EndObject(3);
    }

};
}


// The MIT License (MIT)
//
// Copyright (c) 2014 Siyuan Ren (netheril96@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <autojsoncxx/autojsoncxx.hpp>

// The comments are reserved for replacement
// such syntax is chosen so that the template file looks like valid C++

namespace MCUProtocol { struct KeyboardInput_C2S {
 unsigned int keyValue;
bool isPressed;

explicit KeyboardInput_C2S():keyValue(), isPressed() {  }


 
}; }


namespace autojsoncxx {

template <>
class SAXEventHandler< ::MCUProtocol::KeyboardInput_C2S > {
private:
    utility::scoped_ptr<error::ErrorBase> the_error;
    int state;
    int depth;

    SAXEventHandler< unsigned int > handler_0;
SAXEventHandler< bool > handler_1;bool has_keyValue;
bool has_isPressed;

    bool check_depth(const char* type)
    {
        if (depth <= 0) {
            the_error.reset(new error::TypeMismatchError("object", type));
            return false;
        }
        return true;
    }

    const char* current_member_name() const
    {
        switch (state) {
            case 0:
    return "keyValue";
case 1:
    return "isPressed";
        default:
            break;
        }
        return "<UNKNOWN>";
    }

    bool checked_event_forwarding(bool success)
    {
        if (!success)
            the_error.reset(new error::ObjectMemberError(current_member_name()));
        return success;
    }

    void set_missing_required(const char* name)
    {
        if (the_error.empty() || the_error->type() != error::MISSING_REQUIRED)
            the_error.reset(new error::RequiredFieldMissingError());

        std::vector<std::string>& missing =
            static_cast<error::RequiredFieldMissingError*>(the_error.get())->missing_members();

        missing.push_back(name);
    }

    void reset_flags()
    {
        has_keyValue = false;
has_isPressed = false;
    }

public:
    explicit SAXEventHandler( ::MCUProtocol::KeyboardInput_C2S * obj)
        : state(-1)
        , depth(0)
        , handler_0(&obj->keyValue)
, handler_1(&obj->isPressed)
    {
        reset_flags();
    }

    bool Null()
    {
        if (!check_depth("null"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Null());

case 1:
    return checked_event_forwarding(handler_1.Null());

        default:
            break;
        }
        return true;
    }

    bool Bool(bool b)
    {
        if (!check_depth("bool"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Bool(b));

case 1:
    return checked_event_forwarding(handler_1.Bool(b));

        default:
            break;
        }
        return true;
    }

    bool Int(int i)
    {
        if (!check_depth("int"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Int(i));

case 1:
    return checked_event_forwarding(handler_1.Int(i));

        default:
            break;
        }
        return true;
    }

    bool Uint(unsigned i)
    {
        if (!check_depth("unsigned"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Uint(i));

case 1:
    return checked_event_forwarding(handler_1.Uint(i));

        default:
            break;
        }
        return true;
    }

    bool Int64(utility::int64_t i)
    {
        if (!check_depth("int64_t"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Int64(i));

case 1:
    return checked_event_forwarding(handler_1.Int64(i));

        default:
            break;
        }
        return true;
    }

    bool Uint64(utility::uint64_t i)
    {
        if (!check_depth("uint64_t"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Uint64(i));

case 1:
    return checked_event_forwarding(handler_1.Uint64(i));

        default:
            break;
        }
        return true;
    }

    bool Double(double d)
    {
        if (!check_depth("double"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Double(d));

case 1:
    return checked_event_forwarding(handler_1.Double(d));

        default:
            break;
        }
        return true;
    }

    bool String(const char* str, SizeType length, bool copy)
    {
        if (!check_depth("string"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.String(str, length, copy));

case 1:
    return checked_event_forwarding(handler_1.String(str, length, copy));

        default:
            break;
        }
        return true;
    }

    bool Key(const char* str, SizeType length, bool copy)
    {
        if (!check_depth("object"))
            return false;

        if (depth == 1) {
            if (0) {
            }
            else if (utility::string_equal(str, length, "\x6b\x65\x79\x56\x61\x6c\x75\x65", 8))
                         { state=0; has_keyValue = true; }
else if (utility::string_equal(str, length, "\x69\x73\x50\x72\x65\x73\x73\x65\x64", 9))
                         { state=1; has_isPressed = true; }
            else {
                state = -1;
                return true;
            }

        } else {
            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.Key(str, length, copy));

case 1:
    return checked_event_forwarding(handler_1.Key(str, length, copy));

            default:
                break;
            }
        }
        return true;
    }

    bool StartArray()
    {
        if (!check_depth("array"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.StartArray());

case 1:
    return checked_event_forwarding(handler_1.StartArray());

        default:
            break;
        }
        return true;
    }

    bool EndArray(SizeType length)
    {
        if (!check_depth("array"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.EndArray(length));

case 1:
    return checked_event_forwarding(handler_1.EndArray(length));

        default:
            break;
        }
        return true;
    }

    bool StartObject()
    {
        ++depth;
        if (depth > 1) {

            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.StartObject());

case 1:
    return checked_event_forwarding(handler_1.StartObject());

            default:
                break;
            }
        }
        return true;
    }

    bool EndObject(SizeType length)
    {
        --depth;
        if (depth > 0) {

            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.EndObject(length));

case 1:
    return checked_event_forwarding(handler_1.EndObject(length));

            default:
                break;
            }
        } else {
            if (!has_keyValue) set_missing_required("keyValue");
if (!has_isPressed) set_missing_required("isPressed");
        }
        return the_error.empty();
    }

    bool HasError() const
    {
        return !this->the_error.empty();
    }

    bool ReapError(error::ErrorStack& errs)
    {
        if (this->the_error.empty())
            return false;

        errs.push(this->the_error.release());

        switch (state) {

        case 0:
     handler_0.ReapError(errs); break;
case 1:
     handler_1.ReapError(errs); break;

        default:
            break;
        }

        return true;
    }

    void PrepareForReuse()
    {
        depth = 0;
        state = -1;
        the_error.reset();
        reset_flags();
        handler_0.PrepareForReuse();
handler_1.PrepareForReuse();

    }
};

template < class Writer835e9d2a71507fdac1104cb2b1486f741384bbbc54ec61a44eadb4854d849455 >
struct Serializer< Writer835e9d2a71507fdac1104cb2b1486f741384bbbc54ec61a44eadb4854d849455, ::MCUProtocol::KeyboardInput_C2S > {

    void operator()( Writer835e9d2a71507fdac1104cb2b1486f741384bbbc54ec61a44eadb4854d849455& w, const ::MCUProtocol::KeyboardInput_C2S& value) const
    {
        w.StartObject();

        w.Key("\x6b\x65\x79\x56\x61\x6c\x75\x65", 8, false); Serializer< Writer835e9d2a71507fdac1104cb2b1486f741384bbbc54ec61a44eadb4854d849455, unsigned int >()(w, value.keyValue);
w.Key("\x69\x73\x50\x72\x65\x73\x73\x65\x64", 9, false); Serializer< Writer835e9d2a71507fdac1104cb2b1486f741384bbbc54ec61a44eadb4854d849455, bool >()(w, value.isPressed);

        w.EndObject(2);
    }

};
}


// The MIT License (MIT)
//
// Copyright (c) 2014 Siyuan Ren (netheril96@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <autojsoncxx/autojsoncxx.hpp>

// The comments are reserved for replacement
// such syntax is chosen so that the template file looks like valid C++

namespace MCUProtocol { struct HeartbeatInfo {
 unsigned int UserNum;
std::vector<std::string> UserList;
std::string Publisher;
std::string Operater;

explicit HeartbeatInfo():UserNum(), UserList(), Publisher(), Operater() {  }


 
}; }


namespace autojsoncxx {

template <>
class SAXEventHandler< ::MCUProtocol::HeartbeatInfo > {
private:
    utility::scoped_ptr<error::ErrorBase> the_error;
    int state;
    int depth;

    SAXEventHandler< unsigned int > handler_0;
SAXEventHandler< std::vector<std::string> > handler_1;
SAXEventHandler< std::string > handler_2;
SAXEventHandler< std::string > handler_3;bool has_UserNum;
bool has_UserList;
bool has_Publisher;
bool has_Operater;

    bool check_depth(const char* type)
    {
        if (depth <= 0) {
            the_error.reset(new error::TypeMismatchError("object", type));
            return false;
        }
        return true;
    }

    const char* current_member_name() const
    {
        switch (state) {
            case 0:
    return "UserNum";
case 1:
    return "UserList";
case 2:
    return "Publisher";
case 3:
    return "Operater";
        default:
            break;
        }
        return "<UNKNOWN>";
    }

    bool checked_event_forwarding(bool success)
    {
        if (!success)
            the_error.reset(new error::ObjectMemberError(current_member_name()));
        return success;
    }

    void set_missing_required(const char* name)
    {
        if (the_error.empty() || the_error->type() != error::MISSING_REQUIRED)
            the_error.reset(new error::RequiredFieldMissingError());

        std::vector<std::string>& missing =
            static_cast<error::RequiredFieldMissingError*>(the_error.get())->missing_members();

        missing.push_back(name);
    }

    void reset_flags()
    {
        has_UserNum = false;
has_UserList = false;
has_Publisher = false;
has_Operater = false;
    }

public:
    explicit SAXEventHandler( ::MCUProtocol::HeartbeatInfo * obj)
        : state(-1)
        , depth(0)
        , handler_0(&obj->UserNum)
, handler_1(&obj->UserList)
, handler_2(&obj->Publisher)
, handler_3(&obj->Operater)
    {
        reset_flags();
    }

    bool Null()
    {
        if (!check_depth("null"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Null());

case 1:
    return checked_event_forwarding(handler_1.Null());

case 2:
    return checked_event_forwarding(handler_2.Null());

case 3:
    return checked_event_forwarding(handler_3.Null());

        default:
            break;
        }
        return true;
    }

    bool Bool(bool b)
    {
        if (!check_depth("bool"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Bool(b));

case 1:
    return checked_event_forwarding(handler_1.Bool(b));

case 2:
    return checked_event_forwarding(handler_2.Bool(b));

case 3:
    return checked_event_forwarding(handler_3.Bool(b));

        default:
            break;
        }
        return true;
    }

    bool Int(int i)
    {
        if (!check_depth("int"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Int(i));

case 1:
    return checked_event_forwarding(handler_1.Int(i));

case 2:
    return checked_event_forwarding(handler_2.Int(i));

case 3:
    return checked_event_forwarding(handler_3.Int(i));

        default:
            break;
        }
        return true;
    }

    bool Uint(unsigned i)
    {
        if (!check_depth("unsigned"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Uint(i));

case 1:
    return checked_event_forwarding(handler_1.Uint(i));

case 2:
    return checked_event_forwarding(handler_2.Uint(i));

case 3:
    return checked_event_forwarding(handler_3.Uint(i));

        default:
            break;
        }
        return true;
    }

    bool Int64(utility::int64_t i)
    {
        if (!check_depth("int64_t"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Int64(i));

case 1:
    return checked_event_forwarding(handler_1.Int64(i));

case 2:
    return checked_event_forwarding(handler_2.Int64(i));

case 3:
    return checked_event_forwarding(handler_3.Int64(i));

        default:
            break;
        }
        return true;
    }

    bool Uint64(utility::uint64_t i)
    {
        if (!check_depth("uint64_t"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Uint64(i));

case 1:
    return checked_event_forwarding(handler_1.Uint64(i));

case 2:
    return checked_event_forwarding(handler_2.Uint64(i));

case 3:
    return checked_event_forwarding(handler_3.Uint64(i));

        default:
            break;
        }
        return true;
    }

    bool Double(double d)
    {
        if (!check_depth("double"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Double(d));

case 1:
    return checked_event_forwarding(handler_1.Double(d));

case 2:
    return checked_event_forwarding(handler_2.Double(d));

case 3:
    return checked_event_forwarding(handler_3.Double(d));

        default:
            break;
        }
        return true;
    }

    bool String(const char* str, SizeType length, bool copy)
    {
        if (!check_depth("string"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.String(str, length, copy));

case 1:
    return checked_event_forwarding(handler_1.String(str, length, copy));

case 2:
    return checked_event_forwarding(handler_2.String(str, length, copy));

case 3:
    return checked_event_forwarding(handler_3.String(str, length, copy));

        default:
            break;
        }
        return true;
    }

    bool Key(const char* str, SizeType length, bool copy)
    {
        if (!check_depth("object"))
            return false;

        if (depth == 1) {
            if (0) {
            }
            else if (utility::string_equal(str, length, "\x55\x73\x65\x72\x4e\x75\x6d", 7))
                         { state=0; has_UserNum = true; }
else if (utility::string_equal(str, length, "\x55\x73\x65\x72\x4c\x69\x73\x74", 8))
                         { state=1; has_UserList = true; }
else if (utility::string_equal(str, length, "\x50\x75\x62\x6c\x69\x73\x68\x65\x72", 9))
                         { state=2; has_Publisher = true; }
else if (utility::string_equal(str, length, "\x4f\x70\x65\x72\x61\x74\x65\x72", 8))
                         { state=3; has_Operater = true; }
            else {
                state = -1;
                return true;
            }

        } else {
            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.Key(str, length, copy));

case 1:
    return checked_event_forwarding(handler_1.Key(str, length, copy));

case 2:
    return checked_event_forwarding(handler_2.Key(str, length, copy));

case 3:
    return checked_event_forwarding(handler_3.Key(str, length, copy));

            default:
                break;
            }
        }
        return true;
    }

    bool StartArray()
    {
        if (!check_depth("array"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.StartArray());

case 1:
    return checked_event_forwarding(handler_1.StartArray());

case 2:
    return checked_event_forwarding(handler_2.StartArray());

case 3:
    return checked_event_forwarding(handler_3.StartArray());

        default:
            break;
        }
        return true;
    }

    bool EndArray(SizeType length)
    {
        if (!check_depth("array"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.EndArray(length));

case 1:
    return checked_event_forwarding(handler_1.EndArray(length));

case 2:
    return checked_event_forwarding(handler_2.EndArray(length));

case 3:
    return checked_event_forwarding(handler_3.EndArray(length));

        default:
            break;
        }
        return true;
    }

    bool StartObject()
    {
        ++depth;
        if (depth > 1) {

            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.StartObject());

case 1:
    return checked_event_forwarding(handler_1.StartObject());

case 2:
    return checked_event_forwarding(handler_2.StartObject());

case 3:
    return checked_event_forwarding(handler_3.StartObject());

            default:
                break;
            }
        }
        return true;
    }

    bool EndObject(SizeType length)
    {
        --depth;
        if (depth > 0) {

            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.EndObject(length));

case 1:
    return checked_event_forwarding(handler_1.EndObject(length));

case 2:
    return checked_event_forwarding(handler_2.EndObject(length));

case 3:
    return checked_event_forwarding(handler_3.EndObject(length));

            default:
                break;
            }
        } else {
            if (!has_UserNum) set_missing_required("UserNum");
if (!has_UserList) set_missing_required("UserList");
if (!has_Publisher) set_missing_required("Publisher");
if (!has_Operater) set_missing_required("Operater");
        }
        return the_error.empty();
    }

    bool HasError() const
    {
        return !this->the_error.empty();
    }

    bool ReapError(error::ErrorStack& errs)
    {
        if (this->the_error.empty())
            return false;

        errs.push(this->the_error.release());

        switch (state) {

        case 0:
     handler_0.ReapError(errs); break;
case 1:
     handler_1.ReapError(errs); break;
case 2:
     handler_2.ReapError(errs); break;
case 3:
     handler_3.ReapError(errs); break;

        default:
            break;
        }

        return true;
    }

    void PrepareForReuse()
    {
        depth = 0;
        state = -1;
        the_error.reset();
        reset_flags();
        handler_0.PrepareForReuse();
handler_1.PrepareForReuse();
handler_2.PrepareForReuse();
handler_3.PrepareForReuse();

    }
};

template < class Writerb4926d40dbc5dc94c08012a090e00255b0ee2698b142ec903be263bdfad6dd3f >
struct Serializer< Writerb4926d40dbc5dc94c08012a090e00255b0ee2698b142ec903be263bdfad6dd3f, ::MCUProtocol::HeartbeatInfo > {

    void operator()( Writerb4926d40dbc5dc94c08012a090e00255b0ee2698b142ec903be263bdfad6dd3f& w, const ::MCUProtocol::HeartbeatInfo& value) const
    {
        w.StartObject();

        w.Key("\x55\x73\x65\x72\x4e\x75\x6d", 7, false); Serializer< Writerb4926d40dbc5dc94c08012a090e00255b0ee2698b142ec903be263bdfad6dd3f, unsigned int >()(w, value.UserNum);
w.Key("\x55\x73\x65\x72\x4c\x69\x73\x74", 8, false); Serializer< Writerb4926d40dbc5dc94c08012a090e00255b0ee2698b142ec903be263bdfad6dd3f, std::vector<std::string> >()(w, value.UserList);
w.Key("\x50\x75\x62\x6c\x69\x73\x68\x65\x72", 9, false); Serializer< Writerb4926d40dbc5dc94c08012a090e00255b0ee2698b142ec903be263bdfad6dd3f, std::string >()(w, value.Publisher);
w.Key("\x4f\x70\x65\x72\x61\x74\x65\x72", 8, false); Serializer< Writerb4926d40dbc5dc94c08012a090e00255b0ee2698b142ec903be263bdfad6dd3f, std::string >()(w, value.Operater);

        w.EndObject(4);
    }

};
}


// The MIT License (MIT)
//
// Copyright (c) 2014 Siyuan Ren (netheril96@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <autojsoncxx/autojsoncxx.hpp>

// The comments are reserved for replacement
// such syntax is chosen so that the template file looks like valid C++

namespace MCUProtocol { struct CursorShapeUpdate_S2C {
 int xhot;
int yhot;
int width;
int height;
int color_size;
std::string color_bytes_base64;
int mask_size;
std::string mask_bytes_base64;

explicit CursorShapeUpdate_S2C():xhot(), yhot(), width(), height(), color_size(), color_bytes_base64(), mask_size(), mask_bytes_base64() {  }


 
}; }


namespace autojsoncxx {

template <>
class SAXEventHandler< ::MCUProtocol::CursorShapeUpdate_S2C > {
private:
    utility::scoped_ptr<error::ErrorBase> the_error;
    int state;
    int depth;

    SAXEventHandler< int > handler_0;
SAXEventHandler< int > handler_1;
SAXEventHandler< int > handler_2;
SAXEventHandler< int > handler_3;
SAXEventHandler< int > handler_4;
SAXEventHandler< std::string > handler_5;
SAXEventHandler< int > handler_6;
SAXEventHandler< std::string > handler_7;bool has_xhot;
bool has_yhot;
bool has_width;
bool has_height;
bool has_color_size;
bool has_color_bytes_base64;

    bool check_depth(const char* type)
    {
        if (depth <= 0) {
            the_error.reset(new error::TypeMismatchError("object", type));
            return false;
        }
        return true;
    }

    const char* current_member_name() const
    {
        switch (state) {
            case 0:
    return "xhot";
case 1:
    return "yhot";
case 2:
    return "width";
case 3:
    return "height";
case 4:
    return "color_size";
case 5:
    return "color_bytes_base64";
case 6:
    return "mask_size";
case 7:
    return "mask_bytes_base64";
        default:
            break;
        }
        return "<UNKNOWN>";
    }

    bool checked_event_forwarding(bool success)
    {
        if (!success)
            the_error.reset(new error::ObjectMemberError(current_member_name()));
        return success;
    }

    void set_missing_required(const char* name)
    {
        if (the_error.empty() || the_error->type() != error::MISSING_REQUIRED)
            the_error.reset(new error::RequiredFieldMissingError());

        std::vector<std::string>& missing =
            static_cast<error::RequiredFieldMissingError*>(the_error.get())->missing_members();

        missing.push_back(name);
    }

    void reset_flags()
    {
        has_xhot = false;
has_yhot = false;
has_width = false;
has_height = false;
has_color_size = false;
has_color_bytes_base64 = false;


    }

public:
    explicit SAXEventHandler( ::MCUProtocol::CursorShapeUpdate_S2C * obj)
        : state(-1)
        , depth(0)
        , handler_0(&obj->xhot)
, handler_1(&obj->yhot)
, handler_2(&obj->width)
, handler_3(&obj->height)
, handler_4(&obj->color_size)
, handler_5(&obj->color_bytes_base64)
, handler_6(&obj->mask_size)
, handler_7(&obj->mask_bytes_base64)
    {
        reset_flags();
    }

    bool Null()
    {
        if (!check_depth("null"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Null());

case 1:
    return checked_event_forwarding(handler_1.Null());

case 2:
    return checked_event_forwarding(handler_2.Null());

case 3:
    return checked_event_forwarding(handler_3.Null());

case 4:
    return checked_event_forwarding(handler_4.Null());

case 5:
    return checked_event_forwarding(handler_5.Null());

case 6:
    return checked_event_forwarding(handler_6.Null());

case 7:
    return checked_event_forwarding(handler_7.Null());

        default:
            break;
        }
        return true;
    }

    bool Bool(bool b)
    {
        if (!check_depth("bool"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Bool(b));

case 1:
    return checked_event_forwarding(handler_1.Bool(b));

case 2:
    return checked_event_forwarding(handler_2.Bool(b));

case 3:
    return checked_event_forwarding(handler_3.Bool(b));

case 4:
    return checked_event_forwarding(handler_4.Bool(b));

case 5:
    return checked_event_forwarding(handler_5.Bool(b));

case 6:
    return checked_event_forwarding(handler_6.Bool(b));

case 7:
    return checked_event_forwarding(handler_7.Bool(b));

        default:
            break;
        }
        return true;
    }

    bool Int(int i)
    {
        if (!check_depth("int"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Int(i));

case 1:
    return checked_event_forwarding(handler_1.Int(i));

case 2:
    return checked_event_forwarding(handler_2.Int(i));

case 3:
    return checked_event_forwarding(handler_3.Int(i));

case 4:
    return checked_event_forwarding(handler_4.Int(i));

case 5:
    return checked_event_forwarding(handler_5.Int(i));

case 6:
    return checked_event_forwarding(handler_6.Int(i));

case 7:
    return checked_event_forwarding(handler_7.Int(i));

        default:
            break;
        }
        return true;
    }

    bool Uint(unsigned i)
    {
        if (!check_depth("unsigned"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Uint(i));

case 1:
    return checked_event_forwarding(handler_1.Uint(i));

case 2:
    return checked_event_forwarding(handler_2.Uint(i));

case 3:
    return checked_event_forwarding(handler_3.Uint(i));

case 4:
    return checked_event_forwarding(handler_4.Uint(i));

case 5:
    return checked_event_forwarding(handler_5.Uint(i));

case 6:
    return checked_event_forwarding(handler_6.Uint(i));

case 7:
    return checked_event_forwarding(handler_7.Uint(i));

        default:
            break;
        }
        return true;
    }

    bool Int64(utility::int64_t i)
    {
        if (!check_depth("int64_t"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Int64(i));

case 1:
    return checked_event_forwarding(handler_1.Int64(i));

case 2:
    return checked_event_forwarding(handler_2.Int64(i));

case 3:
    return checked_event_forwarding(handler_3.Int64(i));

case 4:
    return checked_event_forwarding(handler_4.Int64(i));

case 5:
    return checked_event_forwarding(handler_5.Int64(i));

case 6:
    return checked_event_forwarding(handler_6.Int64(i));

case 7:
    return checked_event_forwarding(handler_7.Int64(i));

        default:
            break;
        }
        return true;
    }

    bool Uint64(utility::uint64_t i)
    {
        if (!check_depth("uint64_t"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Uint64(i));

case 1:
    return checked_event_forwarding(handler_1.Uint64(i));

case 2:
    return checked_event_forwarding(handler_2.Uint64(i));

case 3:
    return checked_event_forwarding(handler_3.Uint64(i));

case 4:
    return checked_event_forwarding(handler_4.Uint64(i));

case 5:
    return checked_event_forwarding(handler_5.Uint64(i));

case 6:
    return checked_event_forwarding(handler_6.Uint64(i));

case 7:
    return checked_event_forwarding(handler_7.Uint64(i));

        default:
            break;
        }
        return true;
    }

    bool Double(double d)
    {
        if (!check_depth("double"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Double(d));

case 1:
    return checked_event_forwarding(handler_1.Double(d));

case 2:
    return checked_event_forwarding(handler_2.Double(d));

case 3:
    return checked_event_forwarding(handler_3.Double(d));

case 4:
    return checked_event_forwarding(handler_4.Double(d));

case 5:
    return checked_event_forwarding(handler_5.Double(d));

case 6:
    return checked_event_forwarding(handler_6.Double(d));

case 7:
    return checked_event_forwarding(handler_7.Double(d));

        default:
            break;
        }
        return true;
    }

    bool String(const char* str, SizeType length, bool copy)
    {
        if (!check_depth("string"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.String(str, length, copy));

case 1:
    return checked_event_forwarding(handler_1.String(str, length, copy));

case 2:
    return checked_event_forwarding(handler_2.String(str, length, copy));

case 3:
    return checked_event_forwarding(handler_3.String(str, length, copy));

case 4:
    return checked_event_forwarding(handler_4.String(str, length, copy));

case 5:
    return checked_event_forwarding(handler_5.String(str, length, copy));

case 6:
    return checked_event_forwarding(handler_6.String(str, length, copy));

case 7:
    return checked_event_forwarding(handler_7.String(str, length, copy));

        default:
            break;
        }
        return true;
    }

    bool Key(const char* str, SizeType length, bool copy)
    {
        if (!check_depth("object"))
            return false;

        if (depth == 1) {
            if (0) {
            }
            else if (utility::string_equal(str, length, "\x78\x68\x6f\x74", 4))
                         { state=0; has_xhot = true; }
else if (utility::string_equal(str, length, "\x79\x68\x6f\x74", 4))
                         { state=1; has_yhot = true; }
else if (utility::string_equal(str, length, "\x77\x69\x64\x74\x68", 5))
                         { state=2; has_width = true; }
else if (utility::string_equal(str, length, "\x68\x65\x69\x67\x68\x74", 6))
                         { state=3; has_height = true; }
else if (utility::string_equal(str, length, "\x63\x6f\x6c\x6f\x72\x5f\x73\x69\x7a\x65", 10))
                         { state=4; has_color_size = true; }
else if (utility::string_equal(str, length, "\x63\x6f\x6c\x6f\x72\x5f\x62\x79\x74\x65\x73\x5f\x62\x61\x73\x65\x36\x34", 18))
                         { state=5; has_color_bytes_base64 = true; }
else if (utility::string_equal(str, length, "\x6d\x61\x73\x6b\x5f\x73\x69\x7a\x65", 9))
                         { state=6;  }
else if (utility::string_equal(str, length, "\x6d\x61\x73\x6b\x5f\x62\x79\x74\x65\x73\x5f\x62\x61\x73\x65\x36\x34", 17))
                         { state=7;  }
            else {
                state = -1;
                return true;
            }

        } else {
            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.Key(str, length, copy));

case 1:
    return checked_event_forwarding(handler_1.Key(str, length, copy));

case 2:
    return checked_event_forwarding(handler_2.Key(str, length, copy));

case 3:
    return checked_event_forwarding(handler_3.Key(str, length, copy));

case 4:
    return checked_event_forwarding(handler_4.Key(str, length, copy));

case 5:
    return checked_event_forwarding(handler_5.Key(str, length, copy));

case 6:
    return checked_event_forwarding(handler_6.Key(str, length, copy));

case 7:
    return checked_event_forwarding(handler_7.Key(str, length, copy));

            default:
                break;
            }
        }
        return true;
    }

    bool StartArray()
    {
        if (!check_depth("array"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.StartArray());

case 1:
    return checked_event_forwarding(handler_1.StartArray());

case 2:
    return checked_event_forwarding(handler_2.StartArray());

case 3:
    return checked_event_forwarding(handler_3.StartArray());

case 4:
    return checked_event_forwarding(handler_4.StartArray());

case 5:
    return checked_event_forwarding(handler_5.StartArray());

case 6:
    return checked_event_forwarding(handler_6.StartArray());

case 7:
    return checked_event_forwarding(handler_7.StartArray());

        default:
            break;
        }
        return true;
    }

    bool EndArray(SizeType length)
    {
        if (!check_depth("array"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.EndArray(length));

case 1:
    return checked_event_forwarding(handler_1.EndArray(length));

case 2:
    return checked_event_forwarding(handler_2.EndArray(length));

case 3:
    return checked_event_forwarding(handler_3.EndArray(length));

case 4:
    return checked_event_forwarding(handler_4.EndArray(length));

case 5:
    return checked_event_forwarding(handler_5.EndArray(length));

case 6:
    return checked_event_forwarding(handler_6.EndArray(length));

case 7:
    return checked_event_forwarding(handler_7.EndArray(length));

        default:
            break;
        }
        return true;
    }

    bool StartObject()
    {
        ++depth;
        if (depth > 1) {

            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.StartObject());

case 1:
    return checked_event_forwarding(handler_1.StartObject());

case 2:
    return checked_event_forwarding(handler_2.StartObject());

case 3:
    return checked_event_forwarding(handler_3.StartObject());

case 4:
    return checked_event_forwarding(handler_4.StartObject());

case 5:
    return checked_event_forwarding(handler_5.StartObject());

case 6:
    return checked_event_forwarding(handler_6.StartObject());

case 7:
    return checked_event_forwarding(handler_7.StartObject());

            default:
                break;
            }
        }
        return true;
    }

    bool EndObject(SizeType length)
    {
        --depth;
        if (depth > 0) {

            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.EndObject(length));

case 1:
    return checked_event_forwarding(handler_1.EndObject(length));

case 2:
    return checked_event_forwarding(handler_2.EndObject(length));

case 3:
    return checked_event_forwarding(handler_3.EndObject(length));

case 4:
    return checked_event_forwarding(handler_4.EndObject(length));

case 5:
    return checked_event_forwarding(handler_5.EndObject(length));

case 6:
    return checked_event_forwarding(handler_6.EndObject(length));

case 7:
    return checked_event_forwarding(handler_7.EndObject(length));

            default:
                break;
            }
        } else {
            if (!has_xhot) set_missing_required("xhot");
if (!has_yhot) set_missing_required("yhot");
if (!has_width) set_missing_required("width");
if (!has_height) set_missing_required("height");
if (!has_color_size) set_missing_required("color_size");
if (!has_color_bytes_base64) set_missing_required("color_bytes_base64");
        }
        return the_error.empty();
    }

    bool HasError() const
    {
        return !this->the_error.empty();
    }

    bool ReapError(error::ErrorStack& errs)
    {
        if (this->the_error.empty())
            return false;

        errs.push(this->the_error.release());

        switch (state) {

        case 0:
     handler_0.ReapError(errs); break;
case 1:
     handler_1.ReapError(errs); break;
case 2:
     handler_2.ReapError(errs); break;
case 3:
     handler_3.ReapError(errs); break;
case 4:
     handler_4.ReapError(errs); break;
case 5:
     handler_5.ReapError(errs); break;
case 6:
     handler_6.ReapError(errs); break;
case 7:
     handler_7.ReapError(errs); break;

        default:
            break;
        }

        return true;
    }

    void PrepareForReuse()
    {
        depth = 0;
        state = -1;
        the_error.reset();
        reset_flags();
        handler_0.PrepareForReuse();
handler_1.PrepareForReuse();
handler_2.PrepareForReuse();
handler_3.PrepareForReuse();
handler_4.PrepareForReuse();
handler_5.PrepareForReuse();
handler_6.PrepareForReuse();
handler_7.PrepareForReuse();

    }
};

template < class Writera5ed756aab3ecfbac8697fcc20ed45f575bb006e5f32a654c689ae8fd133c344 >
struct Serializer< Writera5ed756aab3ecfbac8697fcc20ed45f575bb006e5f32a654c689ae8fd133c344, ::MCUProtocol::CursorShapeUpdate_S2C > {

    void operator()( Writera5ed756aab3ecfbac8697fcc20ed45f575bb006e5f32a654c689ae8fd133c344& w, const ::MCUProtocol::CursorShapeUpdate_S2C& value) const
    {
        w.StartObject();

        w.Key("\x78\x68\x6f\x74", 4, false); Serializer< Writera5ed756aab3ecfbac8697fcc20ed45f575bb006e5f32a654c689ae8fd133c344, int >()(w, value.xhot);
w.Key("\x79\x68\x6f\x74", 4, false); Serializer< Writera5ed756aab3ecfbac8697fcc20ed45f575bb006e5f32a654c689ae8fd133c344, int >()(w, value.yhot);
w.Key("\x77\x69\x64\x74\x68", 5, false); Serializer< Writera5ed756aab3ecfbac8697fcc20ed45f575bb006e5f32a654c689ae8fd133c344, int >()(w, value.width);
w.Key("\x68\x65\x69\x67\x68\x74", 6, false); Serializer< Writera5ed756aab3ecfbac8697fcc20ed45f575bb006e5f32a654c689ae8fd133c344, int >()(w, value.height);
w.Key("\x63\x6f\x6c\x6f\x72\x5f\x73\x69\x7a\x65", 10, false); Serializer< Writera5ed756aab3ecfbac8697fcc20ed45f575bb006e5f32a654c689ae8fd133c344, int >()(w, value.color_size);
w.Key("\x63\x6f\x6c\x6f\x72\x5f\x62\x79\x74\x65\x73\x5f\x62\x61\x73\x65\x36\x34", 18, false); Serializer< Writera5ed756aab3ecfbac8697fcc20ed45f575bb006e5f32a654c689ae8fd133c344, std::string >()(w, value.color_bytes_base64);
w.Key("\x6d\x61\x73\x6b\x5f\x73\x69\x7a\x65", 9, false); Serializer< Writera5ed756aab3ecfbac8697fcc20ed45f575bb006e5f32a654c689ae8fd133c344, int >()(w, value.mask_size);
w.Key("\x6d\x61\x73\x6b\x5f\x62\x79\x74\x65\x73\x5f\x62\x61\x73\x65\x36\x34", 17, false); Serializer< Writera5ed756aab3ecfbac8697fcc20ed45f575bb006e5f32a654c689ae8fd133c344, std::string >()(w, value.mask_bytes_base64);

        w.EndObject(8);
    }

};
}


// The MIT License (MIT)
//
// Copyright (c) 2014 Siyuan Ren (netheril96@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <autojsoncxx/autojsoncxx.hpp>

// The comments are reserved for replacement
// such syntax is chosen so that the template file looks like valid C++

namespace MCUProtocol { struct ClipboardData {
 int data_type;
std::string data;

explicit ClipboardData():data_type(), data() {  }


 
}; }


namespace autojsoncxx {

template <>
class SAXEventHandler< ::MCUProtocol::ClipboardData > {
private:
    utility::scoped_ptr<error::ErrorBase> the_error;
    int state;
    int depth;

    SAXEventHandler< int > handler_0;
SAXEventHandler< std::string > handler_1;bool has_data_type;
bool has_data;

    bool check_depth(const char* type)
    {
        if (depth <= 0) {
            the_error.reset(new error::TypeMismatchError("object", type));
            return false;
        }
        return true;
    }

    const char* current_member_name() const
    {
        switch (state) {
            case 0:
    return "data_type";
case 1:
    return "data";
        default:
            break;
        }
        return "<UNKNOWN>";
    }

    bool checked_event_forwarding(bool success)
    {
        if (!success)
            the_error.reset(new error::ObjectMemberError(current_member_name()));
        return success;
    }

    void set_missing_required(const char* name)
    {
        if (the_error.empty() || the_error->type() != error::MISSING_REQUIRED)
            the_error.reset(new error::RequiredFieldMissingError());

        std::vector<std::string>& missing =
            static_cast<error::RequiredFieldMissingError*>(the_error.get())->missing_members();

        missing.push_back(name);
    }

    void reset_flags()
    {
        has_data_type = false;
has_data = false;
    }

public:
    explicit SAXEventHandler( ::MCUProtocol::ClipboardData * obj)
        : state(-1)
        , depth(0)
        , handler_0(&obj->data_type)
, handler_1(&obj->data)
    {
        reset_flags();
    }

    bool Null()
    {
        if (!check_depth("null"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Null());

case 1:
    return checked_event_forwarding(handler_1.Null());

        default:
            break;
        }
        return true;
    }

    bool Bool(bool b)
    {
        if (!check_depth("bool"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Bool(b));

case 1:
    return checked_event_forwarding(handler_1.Bool(b));

        default:
            break;
        }
        return true;
    }

    bool Int(int i)
    {
        if (!check_depth("int"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Int(i));

case 1:
    return checked_event_forwarding(handler_1.Int(i));

        default:
            break;
        }
        return true;
    }

    bool Uint(unsigned i)
    {
        if (!check_depth("unsigned"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Uint(i));

case 1:
    return checked_event_forwarding(handler_1.Uint(i));

        default:
            break;
        }
        return true;
    }

    bool Int64(utility::int64_t i)
    {
        if (!check_depth("int64_t"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Int64(i));

case 1:
    return checked_event_forwarding(handler_1.Int64(i));

        default:
            break;
        }
        return true;
    }

    bool Uint64(utility::uint64_t i)
    {
        if (!check_depth("uint64_t"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Uint64(i));

case 1:
    return checked_event_forwarding(handler_1.Uint64(i));

        default:
            break;
        }
        return true;
    }

    bool Double(double d)
    {
        if (!check_depth("double"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.Double(d));

case 1:
    return checked_event_forwarding(handler_1.Double(d));

        default:
            break;
        }
        return true;
    }

    bool String(const char* str, SizeType length, bool copy)
    {
        if (!check_depth("string"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.String(str, length, copy));

case 1:
    return checked_event_forwarding(handler_1.String(str, length, copy));

        default:
            break;
        }
        return true;
    }

    bool Key(const char* str, SizeType length, bool copy)
    {
        if (!check_depth("object"))
            return false;

        if (depth == 1) {
            if (0) {
            }
            else if (utility::string_equal(str, length, "\x64\x61\x74\x61\x5f\x74\x79\x70\x65", 9))
                         { state=0; has_data_type = true; }
else if (utility::string_equal(str, length, "\x64\x61\x74\x61", 4))
                         { state=1; has_data = true; }
            else {
                state = -1;
                return true;
            }

        } else {
            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.Key(str, length, copy));

case 1:
    return checked_event_forwarding(handler_1.Key(str, length, copy));

            default:
                break;
            }
        }
        return true;
    }

    bool StartArray()
    {
        if (!check_depth("array"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.StartArray());

case 1:
    return checked_event_forwarding(handler_1.StartArray());

        default:
            break;
        }
        return true;
    }

    bool EndArray(SizeType length)
    {
        if (!check_depth("array"))
            return false;

        switch (state) {

        case 0:
    return checked_event_forwarding(handler_0.EndArray(length));

case 1:
    return checked_event_forwarding(handler_1.EndArray(length));

        default:
            break;
        }
        return true;
    }

    bool StartObject()
    {
        ++depth;
        if (depth > 1) {

            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.StartObject());

case 1:
    return checked_event_forwarding(handler_1.StartObject());

            default:
                break;
            }
        }
        return true;
    }

    bool EndObject(SizeType length)
    {
        --depth;
        if (depth > 0) {

            switch (state) {

            case 0:
    return checked_event_forwarding(handler_0.EndObject(length));

case 1:
    return checked_event_forwarding(handler_1.EndObject(length));

            default:
                break;
            }
        } else {
            if (!has_data_type) set_missing_required("data_type");
if (!has_data) set_missing_required("data");
        }
        return the_error.empty();
    }

    bool HasError() const
    {
        return !this->the_error.empty();
    }

    bool ReapError(error::ErrorStack& errs)
    {
        if (this->the_error.empty())
            return false;

        errs.push(this->the_error.release());

        switch (state) {

        case 0:
     handler_0.ReapError(errs); break;
case 1:
     handler_1.ReapError(errs); break;

        default:
            break;
        }

        return true;
    }

    void PrepareForReuse()
    {
        depth = 0;
        state = -1;
        the_error.reset();
        reset_flags();
        handler_0.PrepareForReuse();
handler_1.PrepareForReuse();

    }
};

template < class Writerf688d425f142c031eb01b3e939ea180bdee2341da5446e779336feae03d4588c >
struct Serializer< Writerf688d425f142c031eb01b3e939ea180bdee2341da5446e779336feae03d4588c, ::MCUProtocol::ClipboardData > {

    void operator()( Writerf688d425f142c031eb01b3e939ea180bdee2341da5446e779336feae03d4588c& w, const ::MCUProtocol::ClipboardData& value) const
    {
        w.StartObject();

        w.Key("\x64\x61\x74\x61\x5f\x74\x79\x70\x65", 9, false); Serializer< Writerf688d425f142c031eb01b3e939ea180bdee2341da5446e779336feae03d4588c, int >()(w, value.data_type);
w.Key("\x64\x61\x74\x61", 4, false); Serializer< Writerf688d425f142c031eb01b3e939ea180bdee2341da5446e779336feae03d4588c, std::string >()(w, value.data);

        w.EndObject(2);
    }

};
}


