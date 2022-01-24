/**
 * Copyright (c) 2020 ~ 2022 KylinSec Co., Ltd.
 * kiran-screensaver is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */

#ifndef XCB_UTILS_H
#define XCB_UTILS_H

#include <QRect>
#include <QRegion>
#include <QScopedPointer>
#include <QVector>
#include <QX11Info>

#include <xcb/xcb.h>
#include <xcb/composite.h>
#include <xcb/randr.h>
#include <xcb/shm.h>

class TestXcbSizeHints;

template <typename T> using ScopedCPointer = QScopedPointer<T, QScopedPointerPodDeleter>;

namespace Xcb {

typedef xcb_window_t WindowId;

xcb_connection_t *default_connection()
{
    return QX11Info::connection();
}

template <typename Reply,
          typename Cookie,
          typename... Args>
struct WrapperData
{
    /**
     * @brief The type returned by the xcb reply function.
     */
    typedef Reply reply_type;
    /**
     * @brief The type returned by the xcb request function.
     */
    typedef Cookie cookie_type;
    /**
     * @brief Variadic arguments combined as a std::tuple.
     * @internal Used for verifying the arguments.
     */
    typedef std::tuple<Args...> argument_types;
    /**
     * @brief The function pointer definition for the xcb request function.
     */
    typedef Cookie (*request_func)(xcb_connection_t*, Args...);
    /**
     * @brief The function pointer definition for the xcb reply function.
     */
    typedef Reply *(*reply_func)(xcb_connection_t*, Cookie, xcb_generic_error_t**);
    /**
     * @brief Number of variadic arguments.
     * @internal Used for verifying the arguments.
     */
    static constexpr std::size_t argumentCount = sizeof...(Args);
};

/**
 * @brief Partial template specialization for WrapperData with no further arguments.
 *
 * This will be used for xcb requests just taking the xcb_connection_t* argument.
 */
template <typename Reply,
          typename Cookie>
struct WrapperData<Reply, Cookie>
{
    typedef Reply reply_type;
    typedef Cookie cookie_type;
    typedef std::tuple<> argument_types;
    typedef Cookie (*request_func)(xcb_connection_t*);
    typedef Reply *(*reply_func)(xcb_connection_t*, Cookie, xcb_generic_error_t**);
    static constexpr std::size_t argumentCount = 0;
};

/**
 * @brief Abstract base class for the wrapper.
 *
 * This class contains the complete functionality of the Wrapper. It's only an abstract
 * base class to provide partial template specialization for more specific constructors.
 */
template<typename Data>
class AbstractWrapper
{
public:
    typedef typename Data::cookie_type Cookie;
    typedef typename Data::reply_type Reply;
    virtual ~AbstractWrapper() {
        cleanup();
    }
    inline AbstractWrapper &operator=(const AbstractWrapper &other) {
        if (this != &other) {
            // if we had managed a reply, free it
            cleanup();
            // copy members
            m_retrieved = other.m_retrieved;
            m_cookie = other.m_cookie;
            m_window = other.m_window;
            m_reply = other.m_reply;
            // take over the responsibility for the reply pointer
            takeFromOther(const_cast<AbstractWrapper&>(other));
        }
        return *this;
    }

    inline const Reply *operator->() {
        getReply();
        return m_reply;
    }
    inline bool isNull() {
        getReply();
        return m_reply == nullptr;
    }
    inline bool isNull() const {
        const_cast<AbstractWrapper*>(this)->getReply();
        return m_reply == NULL;
    }
    inline operator bool() {
        return !isNull();
    }
    inline operator bool() const {
        return !isNull();
    }
    inline const Reply *data() {
        getReply();
        return m_reply;
    }
    inline const Reply *data() const {
        const_cast<AbstractWrapper*>(this)->getReply();
        return m_reply;
    }
    inline WindowId window() const {
        return m_window;
    }
    inline bool isRetrieved() const {
        return m_retrieved;
    }
    /**
     * Returns the value of the reply pointer referenced by this object. The reply pointer of
     * this object will be reset to null. Calling any method which requires the reply to be valid
     * will crash.
     *
     * Callers of this function take ownership of the pointer.
     */
    inline Reply *take() {
        getReply();
        Reply *ret = m_reply;
        m_reply = nullptr;
        m_window = XCB_WINDOW_NONE;
        return ret;
    }

protected:
    AbstractWrapper()
        : m_retrieved(false)
        , m_window(XCB_WINDOW_NONE)
        , m_reply(nullptr)
    {
        m_cookie.sequence = 0;
    }
    explicit AbstractWrapper(WindowId window, Cookie cookie)
        : m_retrieved(false)
        , m_cookie(cookie)
        , m_window(window)
        , m_reply(nullptr)
    {
    }
    explicit AbstractWrapper(const AbstractWrapper &other)
        : m_retrieved(other.m_retrieved)
        , m_cookie(other.m_cookie)
        , m_window(other.m_window)
        , m_reply(nullptr)
    {
        takeFromOther(const_cast<AbstractWrapper&>(other));
    }
    void getReply() {
        if (m_retrieved || !m_cookie.sequence) {
            return;
        }
        m_reply = Data::replyFunc(default_connection(), m_cookie, nullptr);
        m_retrieved = true;
    }

private:
    inline void cleanup() {
        if (!m_retrieved && m_cookie.sequence) {
            xcb_discard_reply(default_connection(), m_cookie.sequence);
        } else if (m_reply) {
            free(m_reply);
        }
    }
    inline void takeFromOther(AbstractWrapper &other) {
        if (m_retrieved) {
            m_reply = other.take();
        } else {
            //ensure that other object doesn't try to get the reply or discards it in the dtor
            other.m_retrieved = true;
            other.m_window = XCB_WINDOW_NONE;
        }
    }
    bool m_retrieved;
    Cookie m_cookie;
    WindowId m_window;
    Reply *m_reply;
};

/**
 * @brief Template to compare the arguments of two std::tuple.
 *
 * @internal Used by static_assert in Wrapper
 */
template <typename T1, typename T2, std::size_t I>
struct tupleCompare
{
    typedef typename std::tuple_element<I, T1>::type tuple1Type;
    typedef typename std::tuple_element<I, T2>::type tuple2Type;
    /**
     * @c true if both tuple have the same arguments, @c false otherwise.
     */
    static constexpr bool value = std::is_same< tuple1Type, tuple2Type >::value && tupleCompare<T1, T2, I-1>::value;
};

/**
 * @brief Recursive template case for first tuple element.
 */
template <typename T1, typename T2>
struct tupleCompare<T1, T2, 0>
{
    typedef typename std::tuple_element<0, T1>::type tuple1Type;
    typedef typename std::tuple_element<0, T2>::type tuple2Type;
    static constexpr bool value = std::is_same< tuple1Type, tuple2Type >::value;
};

/**
 * @brief Wrapper taking a WrapperData as first template argument and xcb request args as variadic args.
 */
template<typename Data, typename... Args>
class Wrapper : public AbstractWrapper<Data>
{
public:
    static_assert(!std::is_same<Data, Xcb::WrapperData<typename Data::reply_type, typename Data::cookie_type, Args...> >::value,
                  "Data template argument must be derived from WrapperData");
    static_assert(std::is_base_of<Xcb::WrapperData<typename Data::reply_type, typename Data::cookie_type, Args...>, Data>::value,
                  "Data template argument must be derived from WrapperData");
    static_assert(sizeof...(Args) == Data::argumentCount,
                    "Wrapper and WrapperData need to have same template argument count");
    static_assert(tupleCompare<std::tuple<Args...>, typename Data::argument_types, sizeof...(Args) - 1>::value,
                    "Argument miss-match between Wrapper and WrapperData");
    Wrapper() = default;
    explicit Wrapper(Args... args)
        : AbstractWrapper<Data>(XCB_WINDOW_NONE, Data::requestFunc(default_connection(), args...))
    {
    }
    explicit Wrapper(xcb_window_t w, Args... args)
        : AbstractWrapper<Data>(w, Data::requestFunc(default_connection(), args...))
    {
    }
};

/**
 * @brief Template specialization for xcb_window_t being first variadic argument.
 */
template<typename Data, typename... Args>
class Wrapper<Data, xcb_window_t, Args...> : public AbstractWrapper<Data>
{
public:
    static_assert(!std::is_same<Data, Xcb::WrapperData<typename Data::reply_type, typename Data::cookie_type, xcb_window_t, Args...> >::value,
                  "Data template argument must be derived from WrapperData");
    static_assert(std::is_base_of<Xcb::WrapperData<typename Data::reply_type, typename Data::cookie_type, xcb_window_t, Args...>, Data>::value,
                  "Data template argument must be derived from WrapperData");
    static_assert(sizeof...(Args) + 1 == Data::argumentCount,
                    "Wrapper and WrapperData need to have same template argument count");
    static_assert(tupleCompare<std::tuple<xcb_window_t, Args...>, typename Data::argument_types, sizeof...(Args)>::value,
                    "Argument miss-match between Wrapper and WrapperData");
    Wrapper() = default;
    explicit Wrapper(xcb_window_t w, Args... args)
        : AbstractWrapper<Data>(w, Data::requestFunc(default_connection(), w, args...))
    {
    }
};

/**
 * @brief Template specialization for no variadic arguments.
 *
 * It's needed to prevent ambiguous constructors being generated.
 */
template<typename Data>
class Wrapper<Data> : public AbstractWrapper<Data>
{
public:
    static_assert(!std::is_same<Data, Xcb::WrapperData<typename Data::reply_type, typename Data::cookie_type> >::value,
                  "Data template argument must be derived from WrapperData");
    static_assert(std::is_base_of<Xcb::WrapperData<typename Data::reply_type, typename Data::cookie_type>, Data>::value,
                  "Data template argument must be derived from WrapperData");
    static_assert(Data::argumentCount == 0, "Wrapper for no arguments constructed with WrapperData with arguments");
    explicit Wrapper()
        : AbstractWrapper<Data>(XCB_WINDOW_NONE, Data::requestFunc(default_connection()))
    {
    }
};

/**
 * @brief Macro to create the WrapperData subclass.
 *
 * Creates a struct with name @p __NAME__ for the xcb request identified by @p __REQUEST__.
 * The variadic arguments are used to pass as template arguments to the WrapperData.
 *
 * The @p __REQUEST__ is the common prefix of the foreign_cookie type, reply type, request function and
 * reply function. E.g. "xcb_get_geometry" is used to create:
 * @li foreign_cookie type xcb_get_geometry_cookie_t
 * @li reply type xcb_get_geometry_reply_t
 * @li request function pointer xcb_get_geometry_unchecked
 * @li reply function pointer xcb_get_geometry_reply
 *
 * @param __NAME__ The name of the WrapperData subclass
 * @param __REQUEST__ The name of the xcb request, e.g. xcb_get_geometry
 * @param __VA_ARGS__ The variadic template arguments, e.g. xcb_drawable_t
 * @see XCB_WRAPPER
 */
#define XCB_WRAPPER_DATA( __NAME__, __REQUEST__, ... ) \
    struct __NAME__ : public WrapperData< __REQUEST__##_reply_t, __REQUEST__##_cookie_t, __VA_ARGS__ > \
    { \
        static constexpr request_func requestFunc = &__REQUEST__##_unchecked; \
        static constexpr reply_func replyFunc = &__REQUEST__##_reply; \
    };

/**
 * @brief Macro to create Wrapper typedef and WrapperData.
 *
 * This macro expands the XCB_WRAPPER_DATA macro and creates an additional
 * typedef for Wrapper with name @p __NAME__. The created WrapperData is also derived
 * from @p __NAME__ with "Data" as suffix.
 *
 * @param __NAME__ The name for the Wrapper typedef
 * @param __REQUEST__ The name of the xcb request, passed to XCB_WRAPPER_DATA
 * @param __VA_ARGS__ The variadic template arguments for Wrapper and WrapperData
 * @see XCB_WRAPPER_DATA
 */
#define XCB_WRAPPER( __NAME__, __REQUEST__, ... ) \
    XCB_WRAPPER_DATA( __NAME__##Data, __REQUEST__, __VA_ARGS__ ) \
    typedef Wrapper< __NAME__##Data, __VA_ARGS__ > __NAME__;

namespace Randr{
XCB_WRAPPER(ScreenInfo, xcb_randr_get_screen_info, xcb_window_t)
XCB_WRAPPER_DATA(ScreenResourcesData, xcb_randr_get_screen_resources, xcb_window_t)
class ScreenResources : public Wrapper<ScreenResourcesData, xcb_window_t>
{
public:
    explicit ScreenResources(WindowId window) : Wrapper<ScreenResourcesData, xcb_window_t>(window) {}

    inline xcb_randr_crtc_t *crtcs() {
        if (isNull()) {
            return nullptr;
        }
        return xcb_randr_get_screen_resources_crtcs(data());
    }
    inline xcb_randr_mode_info_t *modes() {
        if (isNull()) {
            return nullptr;
        }
        return xcb_randr_get_screen_resources_modes(data());
    }
    inline uint8_t *names() {
        if (isNull()) {
            return nullptr;
        }
        return xcb_randr_get_screen_resources_names(data());
    }
};

XCB_WRAPPER_DATA(CrtcGammaData, xcb_randr_get_crtc_gamma, xcb_randr_crtc_t)
class CrtcGamma : public Wrapper<CrtcGammaData, xcb_randr_crtc_t>
{
public:
    explicit CrtcGamma(xcb_randr_crtc_t c) : Wrapper<CrtcGammaData, xcb_randr_crtc_t>(c) {}

    inline uint16_t *red() {
        return xcb_randr_get_crtc_gamma_red(data());
    }
    inline uint16_t *green() {
        return xcb_randr_get_crtc_gamma_green(data());
    }
    inline uint16_t *blue() {
        return xcb_randr_get_crtc_gamma_blue(data());
    }
};

XCB_WRAPPER_DATA(CrtcInfoData, xcb_randr_get_crtc_info, xcb_randr_crtc_t, xcb_timestamp_t)
class CrtcInfo : public Wrapper<CrtcInfoData, xcb_randr_crtc_t, xcb_timestamp_t>
{
public:
    CrtcInfo() = default;
    CrtcInfo(const CrtcInfo&) = default;
    explicit CrtcInfo(xcb_randr_crtc_t c, xcb_timestamp_t t) : Wrapper<CrtcInfoData, xcb_randr_crtc_t, xcb_timestamp_t>(c, t) {}

    inline QRect rect() {
        const CrtcInfoData::reply_type *info = data();
        if (!info || info->num_outputs == 0 || info->mode == XCB_NONE || info->status != XCB_RANDR_SET_CONFIG_SUCCESS) {
            return QRect();
        }
        return QRect(info->x, info->y, info->width, info->height);
    }
    inline xcb_randr_output_t *outputs() {
        const CrtcInfoData::reply_type *info = data();
        if (!info || info->num_outputs == 0 || info->mode == XCB_NONE || info->status != XCB_RANDR_SET_CONFIG_SUCCESS) {
            return nullptr;
        }
        return xcb_randr_get_crtc_info_outputs(info);
    }
};

XCB_WRAPPER_DATA(OutputInfoData, xcb_randr_get_output_info, xcb_randr_output_t, xcb_timestamp_t)
class OutputInfo : public Wrapper<OutputInfoData, xcb_randr_output_t, xcb_timestamp_t>
{
public:
    OutputInfo() = default;
    OutputInfo(const OutputInfo&) = default;
    explicit OutputInfo(xcb_randr_output_t c, xcb_timestamp_t t) : Wrapper<OutputInfoData, xcb_randr_output_t, xcb_timestamp_t>(c, t) {}

    inline QString name() {
        const OutputInfoData::reply_type *info = data();
        if (!info || info->num_crtcs == 0 || info->num_modes == 0 || info->status != XCB_RANDR_SET_CONFIG_SUCCESS) {
            return QString();
        }
        return QString::fromUtf8(reinterpret_cast<char*>(xcb_randr_get_output_info_name(info)), info->name_len);
    }
};

XCB_WRAPPER_DATA(CurrentResourcesData, xcb_randr_get_screen_resources_current, xcb_window_t)
class CurrentResources : public Wrapper<CurrentResourcesData, xcb_window_t>
{
public:
    explicit CurrentResources(WindowId window) : Wrapper<CurrentResourcesData, xcb_window_t>(window) {}

    inline xcb_randr_crtc_t *crtcs() {
        if (isNull()) {
            return nullptr;
        }
        return xcb_randr_get_screen_resources_current_crtcs(data());
    }
    inline xcb_randr_mode_info_t *modes() {
        if (isNull()) {
            return nullptr;
        }
        return xcb_randr_get_screen_resources_current_modes(data());
    }
};
}
} // namespace X11

#endif // XCB_UTILS_H
