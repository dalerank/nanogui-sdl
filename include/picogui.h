/*
    nanogui/nanogui.h -- Pull in *everything* from NanoGUI

    NanoGUI was developed by Wenzel Jakob <wenzel@inf.ethz.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#ifndef PICOGUI_HEADER_INCLUDE
#define PICOGUI_HEADER_INCLUDE

#include <include/common.h>
#include <include/vector2.h>
#include <unordered_map>
#include <functional>
#include <atomic>

NAMESPACE_BEGIN(nanogui)

#ifdef _MSC_VER
#define NANOGUI_SNPRINTF _snprintf
#else
#define NANOGUI_SNPRINTF snprintf
#endif

enum class Alignment : uint8_t {
    Minimum = 0,
    Middle,
    Maximum,
    Fill
};

enum class Orientation {
    Horizontal = 0,
    Vertical
};

/**
 * This is a list of icon codes for the entypo.ttf font
 * by Daniel Bruce
 */
#define ENTYPO_ICON_PHONE                0x1F4DE
#define ENTYPO_ICON_MOBILE               0x1F4F1
#define ENTYPO_ICON_MOUSE                0xE789
#define ENTYPO_ICON_ADDRESS              0xE723
#define ENTYPO_ICON_MAIL                 0x2709
#define ENTYPO_ICON_PAPER_PLANE          0x1F53F
#define ENTYPO_ICON_PENCIL               0x270E
#define ENTYPO_ICON_FEATHER              0x2712
#define ENTYPO_ICON_ATTACH               0x1F4CE
#define ENTYPO_ICON_INBOX                0xE777
#define ENTYPO_ICON_REPLY                0xE712
#define ENTYPO_ICON_REPLY_ALL            0xE713
#define ENTYPO_ICON_FORWARD              0x27A6
#define ENTYPO_ICON_USER                 0x1F464
#define ENTYPO_ICON_USERS                0x1F465
#define ENTYPO_ICON_ADD_USER             0xE700
#define ENTYPO_ICON_VCARD                0xE722
#define ENTYPO_ICON_EXPORT               0xE715
#define ENTYPO_ICON_LOCATION             0xE724
#define ENTYPO_ICON_MAP                  0xE727
#define ENTYPO_ICON_COMPASS              0xE728
#define ENTYPO_ICON_DIRECTION            0x27A2
#define ENTYPO_ICON_HAIR_CROSS           0x1F3AF
#define ENTYPO_ICON_SHARE                0xE73C
#define ENTYPO_ICON_SHAREABLE            0xE73E
#define ENTYPO_ICON_HEART                0x2665
#define ENTYPO_ICON_HEART_EMPTY          0x2661
#define ENTYPO_ICON_STAR                 0x2605
#define ENTYPO_ICON_STAR_EMPTY           0x2606
#define ENTYPO_ICON_THUMBS_UP            0x1F44D
#define ENTYPO_ICON_THUMBS_DOWN          0x1F44E
#define ENTYPO_ICON_CHAT                 0xE720
#define ENTYPO_ICON_COMMENT              0xE718
#define ENTYPO_ICON_QUOTE                0x275E
#define ENTYPO_ICON_HOME                 0x2302
#define ENTYPO_ICON_POPUP                0xE74C
#define ENTYPO_ICON_SEARCH               0x1F50D
#define ENTYPO_ICON_FLASHLIGHT           0x1F526
#define ENTYPO_ICON_PRINT                0xE716
#define ENTYPO_ICON_BELL                 0x1F514
#define ENTYPO_ICON_LINK                 0x1F517
#define ENTYPO_ICON_FLAG                 0x2691
#define ENTYPO_ICON_COG                  0x2699
#define ENTYPO_ICON_TOOLS                0x2692
#define ENTYPO_ICON_TROPHY               0x1F3C6
#define ENTYPO_ICON_TAG                  0xE70C
#define ENTYPO_ICON_CAMERA               0x1F4F7
#define ENTYPO_ICON_MEGAPHONE            0x1F4E3
#define ENTYPO_ICON_MOON                 0x263D
#define ENTYPO_ICON_PALETTE              0x1F3A8
#define ENTYPO_ICON_LEAF                 0x1F342
#define ENTYPO_ICON_NOTE                 0x266A
#define ENTYPO_ICON_BEAMED_NOTE          0x266B
#define ENTYPO_ICON_NEW                  0x1F4A5
#define ENTYPO_ICON_GRADUATION_CAP       0x1F393
#define ENTYPO_ICON_BOOK                 0x1F4D5
#define ENTYPO_ICON_NEWSPAPER            0x1F4F0
#define ENTYPO_ICON_BAG                  0x1F45C
#define ENTYPO_ICON_AIRPLANE             0x2708
#define ENTYPO_ICON_LIFEBUOY             0xE788
#define ENTYPO_ICON_EYE                  0xE70A
#define ENTYPO_ICON_CLOCK                0x1F554
#define ENTYPO_ICON_MIC                  0x1F3A4
#define ENTYPO_ICON_CALENDAR             0x1F4C5
#define ENTYPO_ICON_FLASH                0x26A1
#define ENTYPO_ICON_THUNDER_CLOUD        0x26C8
#define ENTYPO_ICON_DROPLET              0x1F4A7
#define ENTYPO_ICON_CD                   0x1F4BF
#define ENTYPO_ICON_BRIEFCASE            0x1F4BC
#define ENTYPO_ICON_AIR                  0x1F4A8
#define ENTYPO_ICON_HOURGLASS            0x23F3
#define ENTYPO_ICON_GAUGE                0x1F6C7
#define ENTYPO_ICON_LANGUAGE             0x1F394
#define ENTYPO_ICON_NETWORK              0xE776
#define ENTYPO_ICON_KEY                  0x1F511
#define ENTYPO_ICON_BATTERY              0x1F50B
#define ENTYPO_ICON_BUCKET               0x1F4FE
#define ENTYPO_ICON_MAGNET               0xE7A1
#define ENTYPO_ICON_DRIVE                0x1F4FD
#define ENTYPO_ICON_CUP                  0x2615
#define ENTYPO_ICON_ROCKET               0x1F680
#define ENTYPO_ICON_BRUSH                0xE79A
#define ENTYPO_ICON_SUITCASE             0x1F6C6
#define ENTYPO_ICON_TRAFFIC_CONE         0x1F6C8
#define ENTYPO_ICON_GLOBE                0x1F30E
#define ENTYPO_ICON_KEYBOARD             0x2328
#define ENTYPO_ICON_BROWSER              0xE74E
#define ENTYPO_ICON_PUBLISH              0xE74D
#define ENTYPO_ICON_PROGRESS_3           0xE76B
#define ENTYPO_ICON_PROGRESS_2           0xE76A
#define ENTYPO_ICON_PROGRESS_1           0xE769
#define ENTYPO_ICON_PROGRESS_0           0xE768
#define ENTYPO_ICON_LIGHT_DOWN           0x1F505
#define ENTYPO_ICON_LIGHT_UP             0x1F506
#define ENTYPO_ICON_ADJUST               0x25D1
#define ENTYPO_ICON_CODE                 0xE714
#define ENTYPO_ICON_MONITOR              0x1F4BB
#define ENTYPO_ICON_INFINITY             0x221E
#define ENTYPO_ICON_LIGHT_BULB           0x1F4A1
#define ENTYPO_ICON_CREDIT_CARD          0x1F4B3
#define ENTYPO_ICON_DATABASE             0x1F4F8
#define ENTYPO_ICON_VOICEMAIL            0x2707
#define ENTYPO_ICON_CLIPBOARD            0x1F4CB
#define ENTYPO_ICON_CART                 0xE73D
#define ENTYPO_ICON_BOX                  0x1F4E6
#define ENTYPO_ICON_TICKET               0x1F3AB
#define ENTYPO_ICON_RSS                  0xE73A
#define ENTYPO_ICON_SIGNAL               0x1F4F6
#define ENTYPO_ICON_THERMOMETER          0x1F4FF
#define ENTYPO_ICON_WATER                0x1F4A6
#define ENTYPO_ICON_SWEDEN               0xF601
#define ENTYPO_ICON_LINE_GRAPH           0x1F4C8
#define ENTYPO_ICON_PIE_CHART            0x25F4
#define ENTYPO_ICON_BAR_GRAPH            0x1F4CA
#define ENTYPO_ICON_AREA_GRAPH           0x1F53E
#define ENTYPO_ICON_LOCK                 0x1F512
#define ENTYPO_ICON_LOCK_OPEN            0x1F513
#define ENTYPO_ICON_LOGOUT               0xE741
#define ENTYPO_ICON_LOGIN                0xE740
#define ENTYPO_ICON_CHECK                0x2713
#define ENTYPO_ICON_CROSS                0x274C
#define ENTYPO_ICON_SQUARED_MINUS        0x229F
#define ENTYPO_ICON_SQUARED_PLUS         0x229E
#define ENTYPO_ICON_SQUARED_CROSS        0x274E
#define ENTYPO_ICON_CIRCLED_MINUS        0x2296
#define ENTYPO_ICON_CIRCLED_PLUS         0x2295
#define ENTYPO_ICON_CIRCLED_CROSS        0x2716
#define ENTYPO_ICON_MINUS                0x2796
#define ENTYPO_ICON_PLUS                 0x2795
#define ENTYPO_ICON_ERASE                0x232B
#define ENTYPO_ICON_BLOCK                0x1F6AB
#define ENTYPO_ICON_INFO                 0x2139
#define ENTYPO_ICON_CIRCLED_INFO         0xE705
#define ENTYPO_ICON_HELP                 0x2753
#define ENTYPO_ICON_CIRCLED_HELP         0xE704
#define ENTYPO_ICON_WARNING              0x26A0
#define ENTYPO_ICON_CYCLE                0x1F504
#define ENTYPO_ICON_CW                   0x27F3
#define ENTYPO_ICON_CCW                  0x27F2
#define ENTYPO_ICON_SHUFFLE              0x1F500
#define ENTYPO_ICON_BACK                 0x1F519
#define ENTYPO_ICON_LEVEL_DOWN           0x21B3
#define ENTYPO_ICON_RETWEET              0xE717
#define ENTYPO_ICON_LOOP                 0x1F501
#define ENTYPO_ICON_BACK_IN_TIME         0xE771
#define ENTYPO_ICON_LEVEL_UP             0x21B0
#define ENTYPO_ICON_SWITCH               0x21C6
#define ENTYPO_ICON_NUMBERED_LIST        0xE005
#define ENTYPO_ICON_ADD_TO_LIST          0xE003
#define ENTYPO_ICON_LAYOUT               0x268F
#define ENTYPO_ICON_LIST                 0x2630
#define ENTYPO_ICON_TEXT_DOC             0x1F4C4
#define ENTYPO_ICON_TEXT_DOC_INVERTED    0xE731
#define ENTYPO_ICON_DOC                  0xE730
#define ENTYPO_ICON_DOCS                 0xE736
#define ENTYPO_ICON_LANDSCAPE_DOC        0xE737
#define ENTYPO_ICON_PICTURE              0x1F304
#define ENTYPO_ICON_VIDEO                0x1F3AC
#define ENTYPO_ICON_MUSIC                0x1F3B5
#define ENTYPO_ICON_FOLDER               0x1F4C1
#define ENTYPO_ICON_ARCHIVE              0xE800
#define ENTYPO_ICON_TRASH                0xE729
#define ENTYPO_ICON_UPLOAD               0x1F4E4
#define ENTYPO_ICON_DOWNLOAD             0x1F4E5
#define ENTYPO_ICON_SAVE                 0x1F4BE
#define ENTYPO_ICON_INSTALL              0xE778
#define ENTYPO_ICON_CLOUD                0x2601
#define ENTYPO_ICON_UPLOAD_CLOUD         0xE711
#define ENTYPO_ICON_BOOKMARK             0x1F516
#define ENTYPO_ICON_BOOKMARKS            0x1F4D1
#define ENTYPO_ICON_OPEN_BOOK            0x1F4D6
#define ENTYPO_ICON_PLAY                 0x25B6
#define ENTYPO_ICON_PAUS                 0x2016
#define ENTYPO_ICON_RECORD               0x25CF
#define ENTYPO_ICON_STOP                 0x25A0
#define ENTYPO_ICON_FF                   0x23E9
#define ENTYPO_ICON_FB                   0x23EA
#define ENTYPO_ICON_TO_START             0x23EE
#define ENTYPO_ICON_TO_END               0x23ED
#define ENTYPO_ICON_RESIZE_FULL          0xE744
#define ENTYPO_ICON_RESIZE_SMALL         0xE746
#define ENTYPO_ICON_VOLUME               0x23F7
#define ENTYPO_ICON_SOUND                0x1F50A
#define ENTYPO_ICON_MUTE                 0x1F507
#define ENTYPO_ICON_FLOW_CASCADE         0x1F568
#define ENTYPO_ICON_FLOW_BRANCH          0x1F569
#define ENTYPO_ICON_FLOW_TREE            0x1F56A
#define ENTYPO_ICON_FLOW_LINE            0x1F56B
#define ENTYPO_ICON_FLOW_PARALLEL        0x1F56C
#define ENTYPO_ICON_LEFT_BOLD            0xE4AD
#define ENTYPO_ICON_DOWN_BOLD            0xE4B0
#define ENTYPO_ICON_UP_BOLD              0xE4AF
#define ENTYPO_ICON_RIGHT_BOLD           0xE4AE
#define ENTYPO_ICON_LEFT                 0x2B05
#define ENTYPO_ICON_DOWN                 0x2B07
#define ENTYPO_ICON_UP                   0x2B06
#define ENTYPO_ICON_RIGHT                0x27A1
#define ENTYPO_ICON_CIRCLED_LEFT         0xE759
#define ENTYPO_ICON_CIRCLED_DOWN         0xE758
#define ENTYPO_ICON_CIRCLED_UP           0xE75B
#define ENTYPO_ICON_CIRCLED_RIGHT        0xE75A
#define ENTYPO_ICON_TRIANGLE_LEFT        0x25C2
#define ENTYPO_ICON_TRIANGLE_DOWN        0x25BE
#define ENTYPO_ICON_TRIANGLE_UP          0x25B4
#define ENTYPO_ICON_TRIANGLE_RIGHT       0x25B8
#define ENTYPO_ICON_CHEVRON_LEFT         0xE75D
#define ENTYPO_ICON_CHEVRON_DOWN         0xE75C
#define ENTYPO_ICON_CHEVRON_UP           0xE75F
#define ENTYPO_ICON_CHEVRON_RIGHT        0xE75E
#define ENTYPO_ICON_CHEVRON_SMALL_LEFT   0xE761
#define ENTYPO_ICON_CHEVRON_SMALL_DOWN   0xE760
#define ENTYPO_ICON_CHEVRON_SMALL_UP     0xE763
#define ENTYPO_ICON_CHEVRON_SMALL_RIGHT  0xE762
#define ENTYPO_ICON_CHEVRON_THIN_LEFT    0xE765
#define ENTYPO_ICON_CHEVRON_THIN_DOWN    0xE764
#define ENTYPO_ICON_CHEVRON_THIN_UP      0xE767
#define ENTYPO_ICON_CHEVRON_THIN_RIGHT   0xE766
#define ENTYPO_ICON_LEFT_THIN            0x2190
#define ENTYPO_ICON_DOWN_THIN            0x2193
#define ENTYPO_ICON_UP_THIN              0x2191
#define ENTYPO_ICON_RIGHT_THIN           0x2192
#define ENTYPO_ICON_ARROW_COMBO          0xE74F
#define ENTYPO_ICON_THREE_DOTS           0x23F6
#define ENTYPO_ICON_TWO_DOTS             0x23F5
#define ENTYPO_ICON_DOT                  0x23F4
#define ENTYPO_ICON_CC                   0x1F545
#define ENTYPO_ICON_CC_BY                0x1F546
#define ENTYPO_ICON_CC_NC                0x1F547
#define ENTYPO_ICON_CC_NC_EU             0x1F548
#define ENTYPO_ICON_CC_NC_JP             0x1F549
#define ENTYPO_ICON_CC_SA                0x1F54A
#define ENTYPO_ICON_CC_ND                0x1F54B
#define ENTYPO_ICON_CC_PD                0x1F54C
#define ENTYPO_ICON_CC_ZERO              0x1F54D
#define ENTYPO_ICON_CC_SHARE             0x1F54E
#define ENTYPO_ICON_CC_REMIX             0x1F54F
#define ENTYPO_ICON_DB_LOGO              0x1F5F9
#define ENTYPO_ICON_DB_SHAPE             0x1F5FA
#define ENTYPO_ICON_GITHUB               0xF300
#define ENTYPO_ICON_C_GITHUB             0xF301
#define ENTYPO_ICON_FLICKR               0xF303
#define ENTYPO_ICON_C_FLICKR             0xF304
#define ENTYPO_ICON_VIMEO                0xF306
#define ENTYPO_ICON_C_VIMEO              0xF307
#define ENTYPO_ICON_TWITTER              0xF309
#define ENTYPO_ICON_C_TWITTER            0xF30A
#define ENTYPO_ICON_FACEBOOK             0xF30C
#define ENTYPO_ICON_C_FACEBOOK           0xF30D
#define ENTYPO_ICON_S_FACEBOOK           0xF30E
#define ENTYPO_ICON_GOOGLEPLUS           0xF30F
#define ENTYPO_ICON_C_GOOGLEPLUS         0xF310
#define ENTYPO_ICON_PINTEREST            0xF312
#define ENTYPO_ICON_C_PINTEREST          0xF313
#define ENTYPO_ICON_TUMBLR               0xF315
#define ENTYPO_ICON_C_TUMBLR             0xF316
#define ENTYPO_ICON_LINKEDIN             0xF318
#define ENTYPO_ICON_C_LINKEDIN           0xF319
#define ENTYPO_ICON_DRIBBBLE             0xF31B
#define ENTYPO_ICON_C_DRIBBBLE           0xF31C
#define ENTYPO_ICON_STUMBLEUPON          0xF31E
#define ENTYPO_ICON_C_STUMBLEUPON        0xF31F
#define ENTYPO_ICON_LASTFM               0xF321
#define ENTYPO_ICON_C_LASTFM             0xF322
#define ENTYPO_ICON_RDIO                 0xF324
#define ENTYPO_ICON_C_RDIO               0xF325
#define ENTYPO_ICON_SPOTIFY              0xF327
#define ENTYPO_ICON_C_SPOTIFY            0xF328
#define ENTYPO_ICON_QQ                   0xF32A
#define ENTYPO_ICON_INSTAGRAM            0xF32D
#define ENTYPO_ICON_DROPBOX              0xF330
#define ENTYPO_ICON_EVERNOTE             0xF333
#define ENTYPO_ICON_FLATTR               0xF336
#define ENTYPO_ICON_SKYPE                0xF339
#define ENTYPO_ICON_C_SKYPE              0xF33A
#define ENTYPO_ICON_RENREN               0xF33C
#define ENTYPO_ICON_SINA_WEIBO           0xF33F
#define ENTYPO_ICON_PAYPAL               0xF342
#define ENTYPO_ICON_PICASA               0xF345
#define ENTYPO_ICON_SOUNDCLOUD           0xF348
#define ENTYPO_ICON_MIXI                 0xF34B
#define ENTYPO_ICON_BEHANCE              0xF34E
#define ENTYPO_ICON_GOOGLE_CIRCLES       0xF351
#define ENTYPO_ICON_VK                   0xF354
#define ENTYPO_ICON_SMASHING             0xF357

/// Reference counted object base class
class NANOGUI_EXPORT Object {
public:
    /// Default constructor
    Object() { }

    /// Copy constructor
    Object(const Object &) : m_refCount(0) {}

    /// Return the current reference count
    int getRefCount() const { return m_refCount; }

    /// Increase the object's reference count by one
    void incRef() const { ++m_refCount; }

    /** \brief Decrease the reference count of
     * the object and possibly deallocate it.
     *
     * The object will automatically be deallocated once
     * the reference count reaches zero.
     */
    void decRef(bool dealloc = true) const
    {
        --m_refCount;
        if (m_refCount == 0 && dealloc)
            delete this;
        else if (m_refCount < 0)
            throw std::runtime_error("Internal error: reference count < 0!");
    }
protected:
    /** \brief Virtual protected deconstructor.
     * (Will only be called by \ref ref)
     */
    virtual ~Object() { }
private:
    mutable std::atomic<int> m_refCount { 0 };
};

/**
 * \brief Reference counting helper
 *
 * The \a ref refeference template is a simple wrapper to store a
 * pointer to an object. It takes care of increasing and decreasing
 * the reference count of the object. When the last reference goes
 * out of scope, the associated object will be deallocated.
 *
 * \ingroup libcore
 */
template <typename T> class ref {
public:
    /// Create a nullptr reference
    ref() : m_ptr(nullptr) { }

    /// Construct a reference from a pointer
    ref(T *ptr) : m_ptr(ptr) {
        if (m_ptr) ((Object *) m_ptr)->incRef();
    }

    /// Copy constructor
    ref(const ref &r) : m_ptr(r.m_ptr) {
        if (m_ptr)
            ((Object *) m_ptr)->incRef();
    }

    /// Move constructor
    ref(ref &&r) : m_ptr(r.m_ptr) {
        r.m_ptr = nullptr;
    }

    /// Destroy this reference
    ~ref() {
        if (m_ptr)
            ((Object *) m_ptr)->decRef();
    }

    /// Move another reference into the current one
    ref& operator=(ref&& r) {
        if (*this == r)
            return *this;
        if (m_ptr)
            ((Object *) m_ptr)->decRef();
        m_ptr = r.m_ptr;
        r.m_ptr = nullptr;
        return *this;
    }

    /// Overwrite this reference with another reference
    ref& operator=(const ref& r) {
        if (m_ptr == r.m_ptr)
            return *this;
        if (m_ptr)
            ((Object *) m_ptr)->decRef();
        m_ptr = r.m_ptr;
        if (m_ptr)
            ((Object *) m_ptr)->incRef();
        return *this;
    }

    /// Overwrite this reference with a pointer to another object
    ref& operator=(T *ptr) {
        if (m_ptr == ptr)
            return *this;
        if (m_ptr)
            ((Object *) m_ptr)->decRef();
        m_ptr = ptr;
        if (m_ptr)
            ((Object *) m_ptr)->incRef();
        return *this;
    }

    /// Compare this reference with another reference
    bool operator==(const ref &r) const { return m_ptr == r.m_ptr; }

    /// Compare this reference with another reference
    bool operator!=(const ref &r) const { return m_ptr != r.m_ptr; }

    /// Compare this reference with a pointer
    bool operator==(const T* ptr) const { return m_ptr == ptr; }

    /// Compare this reference with a pointer
    bool operator!=(const T* ptr) const { return m_ptr != ptr; }

    /// Access the object referenced by this reference
    T* operator->() { return m_ptr; }

    /// Access the object referenced by this reference
    const T* operator->() const { return m_ptr; }

    /// Return a C++ reference to the referenced object
    T& operator*() { return *m_ptr; }

    /// Return a const C++ reference to the referenced object
    const T& operator*() const { return *m_ptr; }

    /// Return a pointer to the referenced object
    operator T* () { return m_ptr; }

    /// Return a const pointer to the referenced object
    T* get() { return m_ptr; }

    /// Return a pointer to the referenced object
    const T* get() const { return m_ptr; }

    /// Check if the object is defined
    operator bool() const { return m_ptr != nullptr; }
private:
    T *m_ptr;
};

class NANOGUI_EXPORT Theme : public Object {
public:
    Theme(NVGcontext *ctx);

    /* Fonts */
    int mFontNormal;
    int mFontBold;
    int mFontIcons;

    /* Spacing-related parameters */
    int mStandardFontSize;
    int mButtonFontSize;
    int mTextBoxFontSize;
    int mWindowCornerRadius;
    int mWindowHeaderHeight;
    int mWindowDropShadowSize;
    int mButtonCornerRadius;

    /* Generic colors */
    Color mDropShadow;
    Color mTransparent;
    Color mBorderDark;
    Color mBorderLight;
    Color mBorderMedium;
    Color mTextColor;
    Color mDisabledTextColor;
    Color mTextColorShadow;
    Color mIconColor;

    /* Button colors */
    Color mButtonGradientTopFocused;
    Color mButtonGradientBotFocused;
    Color mButtonGradientTopUnfocused;
    Color mButtonGradientBotUnfocused;
    Color mButtonGradientTopPushed;
    Color mButtonGradientBotPushed;

    /* Window colors */
    Color mWindowFillUnfocused;
    Color mWindowFillFocused;
    Color mWindowTitleUnfocused;
    Color mWindowTitleFocused;

    Color mWindowHeaderGradientTop;
    Color mWindowHeaderGradientBot;
    Color mWindowHeaderSepTop;
    Color mWindowHeaderSepBot;

    Color mWindowPopup;
    Color mWindowPopupTransparent;
protected:
    virtual ~Theme() { };
};

/**
 * \brief Base class of all widgets
 *
 * \ref Widget is the base class of all widgets in \c nanogui. It can
 * also be used as an panel to arrange an arbitrary number of child
 * widgets using a layout generator (see \ref Layout).
 */
class NANOGUI_EXPORT Widget : public Object
{
public:
    template<typename WidgetClass, typename... Args>
    WidgetClass& add( const Args&... args)
    {
      WidgetClass* widget = new WidgetClass( this, args... );
      return *widget;
    }

    /// Construct a new widget with the given parent widget
    Widget(Widget *parent);

    /// Return the parent widget
    Widget *parent() { return mParent; }
    /// Return the parent widget
    const Widget *parent() const { return mParent; }
    /// Set the parent widget
    void setParent(Widget *parent) { mParent = parent; }

    /// Return the used \ref Layout generator
    Layout *layout() { return mLayout; }
    /// Return the used \ref Layout generator
    const Layout *layout() const { return mLayout.get(); }
    /// Set the used \ref Layout generator
    void setLayout(Layout *layout) { mLayout = layout; }

    /// Return the \ref Theme used to draw this widget
    Theme *theme() { return mTheme; }
    /// Return the \ref Theme used to draw this widget
    const Theme *theme() const { return mTheme.get(); }
    /// Set the \ref Theme used to draw this widget
    void setTheme(Theme *theme) { mTheme = theme; }

    /// Return the position relative to the parent widget
    const Vector2i &position() const { return mPos; }
    /// Set the position relative to the parent widget
    void setPosition(const Vector2i &pos) { mPos = pos; }

    /// Return the absolute position on screen
    Vector2i absolutePosition() const {
        return mParent ?
            (parent()->absolutePosition() + mPos) : mPos;
    }

    /// Return the size of the widget
    const Vector2i &size() const { return mSize; }
    /// set the size of the widget
    void setSize(const Vector2i &size) { mSize = size; }

    /// Return the width of the widget
    int width() const { return mSize.x(); }
    /// Set the width of the widget
    void setWidth(int width) { mSize.setX(width); }

    /// Return the height of the widget
    int height() const { return mSize.y(); }
    /// Set the height of the widget
    void setHeight(int height) { mSize.setY(height); }

    /**
     * \brief Set the fixed size of this widget
     *
     * If nonzero, components of the fixed size attribute override any values
     * computed by a layout generator associated with this widget. Note that
     * just setting the fixed size alone is not enough to actually change its
     * size; this is done with a call to \ref setSize or a call to \ref performLayout()
     * in the parent widget.
     */
    void setFixedSize(const Vector2i &fixedSize) { mFixedSize = fixedSize; }

    /// Return the fixed size (see \ref setFixedSize())
    const Vector2i &fixedSize() const { return mFixedSize; }

    // Return the fixed width (see \ref setFixedSize())
    int fixedWidth() const { return mFixedSize.x(); }
    // Return the fixed height (see \ref setFixedSize())
    int fixedHeight() const { return mFixedSize.y(); }
    /// Set the fixed width (see \ref setFixedSize())
    void setFixedWidth(int width) { mFixedSize.setX(width); }
    /// Set the fixed height (see \ref setFixedSize())
    void setFixedHeight(int height) { mFixedSize.setY(height); }

    /// Return whether or not the widget is currently visible (assuming all parents are visible)
    bool visible() const { return mVisible; }
    /// Set whether or not the widget is currently visible (assuming all parents are visible)
    void setVisible(bool visible) { mVisible = visible; }

    /// Check if this widget is currently visible, taking parent widgets into account
    bool visibleRecursive() const {
        bool visible = true;
        const Widget *widget = this;
        while (widget) {
            visible &= widget->visible();
            widget = widget->parent();
        }
        return visible;
    }

    /// Return the number of child widgets
    int childCount() const { return (int) mChildren.size(); }

    /// Return the list of child widgets of the current widget
    const std::vector<Widget *> &children() const { return mChildren; }

    /**
     * \brief Add a child widget to the current widget
     *
     * This function almost never needs to be called by hand,
     * since the constructor of \ref Widget automatically
     * adds the current widget to its parent
     */
    void addChild(Widget *widget);

    /// Remove a child widget by index
    void removeChild(int index);

    /// Remove a child widget by value
    void removeChild(const Widget *widget);

    // Walk up the hierarchy and return the parent window
    Window *window();

    /// Associate this widget with an ID value (optional)
    void setId(const std::string &id) { mId = id; }
    /// Return the ID value associated with this widget, if any
    const std::string &id() const { return mId; }

    /// Return whether or not this widget is currently enabled
    bool enabled() const { return mEnabled; }
    /// Set whether or not this widget is currently enabled
    void setEnabled(bool enabled) { mEnabled = enabled; }

    /// Return whether or not this widget is currently focused
    bool focused() const { return mFocused; }
    /// Set whether or not this widget is currently focused
    void setFocused(bool focused) { mFocused = focused; }
    /// Request the focus to be moved to this widget
    void requestFocus();

    const std::string &tooltip() const { return mTooltip; }
    void setTooltip(const std::string &tooltip) { mTooltip = tooltip; }

    /// Return current font size. If not set the default of the current theme will be returned
    int fontSize() const;
    /// Set the font size of this widget
    void setFontSize(int fontSize) { mFontSize = fontSize; }
    /// Return whether the font size is explicitly specified for this widget
    bool hasFontSize() const { return mFontSize > 0; }

    /// Return a pointer to the cursor of the widget
    Cursor cursor() const { return mCursor; }
    /// Set the cursor of the widget
    void setCursor(Cursor cursor) { mCursor = cursor; }

    /// Check if the widget contains a certain position
    bool contains(const Vector2i &p) const {
        auto d = p-mPos;
        return d.x() >= 0 && d.y() >= 0 && d.x() < mSize.x() && d.y() < mSize.y();
    }

    /// Determine the widget located at the given position value (recursive)
    Widget *findWidget(const Vector2i &p);

    /// Handle a mouse button event (default implementation: propagate to children)
    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers);

    /// Handle a mouse motion event (default implementation: propagate to children)
    virtual bool mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers);

    /// Handle a mouse drag event (default implementation: do nothing)
    virtual bool mouseDragEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers);

    /// Handle a mouse enter/leave event (default implementation: record this fact, but do nothing)
    virtual bool mouseEnterEvent(const Vector2i &p, bool enter);

    /// Handle a mouse scroll event (default implementation: propagate to children)
    virtual bool scrollEvent(const Vector2i &p, const Vector2f &rel);

    /// Handle a focus change event (default implementation: record the focus status, but do nothing)
    virtual bool focusEvent(bool focused);

    /// Handle a keyboard event (default implementation: do nothing)
    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers);

    /// Handle text input (UTF-32 format) (default implementation: do nothing)
    virtual bool keyboardCharacterEvent(unsigned int codepoint);

    /// Compute the preferred size of the widget
    virtual Vector2i preferredSize(NVGcontext *ctx) const;

    /// Invoke the associated layout generator to properly place child widgets, if any
    virtual void performLayout(NVGcontext *ctx);

    /// Draw the widget (and all child widgets)
    virtual void draw(NVGcontext *ctx);

    Widget& withPosition( const Vector2i& pos ) { setPosition( pos); return *this; }
    Widget& withFontSize(int size) { setFontSize(size); return *this; }
    Widget& withFixedSize(const Vector2i& size) { setFixedSize(size); return *this; }

    template<typename LayoutClass,typename... Args>
    Widget& withLayout( const Args&... args)
    {
      auto* layout = new LayoutClass( args... );
      setLayout( layout );
      return *this;
    }

protected:
    /// Free all resources used by the widget and any children
    virtual ~Widget();

protected:
    Widget *mParent;
    ref<Theme> mTheme;
    ref<Layout> mLayout;
    std::string mId;
    Vector2i mPos, mSize, mFixedSize;
    std::vector<Widget *> mChildren;
    bool mVisible, mEnabled;
    bool mFocused, mMouseFocus;
    std::string mTooltip;
    int mFontSize;
    Cursor mCursor;
};

class NANOGUI_EXPORT Window : public Widget {
    friend class Popup;
public:
    Window(Widget *parent, const std::string &title = "Untitled");

    /// Return the window title
    const std::string &title() const { return mTitle; }
    /// Set the window title
    void setTitle(const std::string &title) { mTitle = title; }

    /// Is this a model dialog?
    bool modal() const { return mModal; }
    /// Set whether or not this is a modal dialog
    void setModal(bool modal) { mModal = modal; }

    /// Return the panel used to house window buttons
    Widget *buttonPanel();

    /// Dispose the window
    void dispose();

    /// Center the window in the current \ref Screen
    void center();

    /// Draw the window
    virtual void draw(NVGcontext *ctx);

    /// Handle window drag events
    virtual bool mouseDragEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers);
    /// Handle mouse events recursively and bring the current window to the top
    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers);
    /// Accept scroll events and propagate them to the widget under the mouse cursor
    virtual bool scrollEvent(const Vector2i &p, const Vector2f &rel);
    /// Compute the preferred size of the widget
    virtual Vector2i preferredSize(NVGcontext *ctx) const;
    /// Invoke the associated layout generator to properly place child widgets, if any
    virtual void performLayout(NVGcontext *ctx);
protected:
    /// Internal helper function to maintain nested window position values; overridden in \ref Popup
    virtual void refreshRelativePlacement();
protected:
    std::string mTitle;
    Widget *mButtonPanel;
    bool mModal;
    bool mDrag;
};

/**
 * \brief Popup window for combo boxes, popup buttons, nested dialogs etc.
 *
 * Usually the Popup instance is constructed by another widget (e.g. \ref PopupButton)
 * and does not need to be created by hand.
 */
class NANOGUI_EXPORT Popup : public Window {
public:
    /// Create a new popup parented to a screen (first argument) and a parent window
    Popup(Widget *parent, Window *parentWindow);

    /// Return the anchor position in the parent window; the placement of the popup is relative to it
    void setAnchorPos(const Vector2i &anchorPos) { mAnchorPos = anchorPos; }
    /// Set the anchor position in the parent window; the placement of the popup is relative to it
    const Vector2i &anchorPos() const { return mAnchorPos; }

    /// Set the anchor height; this determines the vertical shift relative to the anchor position
    void setAnchorHeight(int anchorHeight) { mAnchorHeight = anchorHeight; }
    /// Return the anchor height; this determines the vertical shift relative to the anchor position
    int anchorHeight() const { return mAnchorHeight; }

    /// Return the parent window of the popup
    Window *parentWindow() { return mParentWindow; }
    /// Return the parent window of the popup
    const Window *parentWindow() const { return mParentWindow; }

    /// Invoke the associated layout generator to properly place child widgets, if any
    virtual void performLayout(NVGcontext *ctx);

    /// Draw the popup window
    virtual void draw(NVGcontext* ctx);
protected:
    /// Internal helper function to maintain nested window position values
    virtual void refreshRelativePlacement();

protected:
    Window *mParentWindow;
    Vector2i mAnchorPos;
    int mAnchorHeight;
};

class NANOGUI_EXPORT Slider : public Widget {
public:
    Slider(Widget *parent);

    float value() const { return mValue; }
    void setValue(float value) { mValue = value; }

    const Color &highlightColor() const { return mHighlightColor; }
    void setHighlightColor(const Color &highlightColor) { mHighlightColor = highlightColor; }

    std::pair<float, float> highlightedRange() const { return mHighlightedRange; }
    void setHighlightedRange(std::pair<float, float> highlightedRange) { mHighlightedRange = highlightedRange; }

    std::function<void(float)> callback() const { return mCallback; }
    void setCallback(const std::function<void(float)> &callback) { mCallback = callback; }

    std::function<void(float)> finalCallback() const { return mFinalCallback; }
    void setFinalCallback(const std::function<void(float)> &callback) { mFinalCallback = callback; }

    virtual Vector2i preferredSize(NVGcontext *ctx) const;
    virtual bool mouseDragEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers);
    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers);
    virtual void draw(NVGcontext* ctx);

protected:
    float mValue;
    std::function<void(float)> mCallback;
    std::function<void(float)> mFinalCallback;
    std::pair<float, float> mHighlightedRange;
    Color mHighlightColor;
};

class NANOGUI_EXPORT Button : public Widget {
public:
    /// Flags to specify the button behavior (can be combined with binary OR)
    enum Flags {
        NormalButton = 1,
        RadioButton  = 2,
        ToggleButton = 4,
        PopupButton  = 8
    };

    enum class IconPosition {
        Left,
        LeftCentered,
        RightCentered,
        Right
    };

    Button(Widget *parent, const std::string& caption = "Untitled", int icon = 0);
    Button(Widget *parent, const std::string& caption,
           const std::function<void()>& callback,
           int icon = 0);

    const std::string &caption() const { return mCaption; }
    void setCaption(const std::string &caption) { mCaption = caption; }

    const Color &backgroundColor() const { return mBackgroundColor; }
    void setBackgroundColor(const Color &backgroundColor) { mBackgroundColor = backgroundColor; }

    const Color &textColor() const { return mTextColor; }
    void setTextColor(const Color &textColor) { mTextColor = textColor; }

    int icon() const { return mIcon; }
    void setIcon(int icon) { mIcon = icon; }

    int flags() const { return mFlags; }
    void setFlags(int buttonFlags) { mFlags = buttonFlags; }

    IconPosition iconPosition() const { return mIconPosition; }
    void setIconPosition(IconPosition iconPosition) { mIconPosition = iconPosition; }

    bool pushed() const { return mPushed; }
    void setPushed(bool pushed) { mPushed = pushed; }

    /// Set the push callback (for any type of button)
    std::function<void()> callback() const { return mCallback; }
    void setCallback(const std::function<void()> &callback) { mCallback = callback; }

    /// Set the change callback (for toggle buttons)
    std::function<void(bool)> changeCallback() const { return mChangeCallback; }
    void setChangeCallback(const std::function<void(bool)>& callback) { mChangeCallback = callback; }

    Button& withCallback(const std::function<void()> &callback) { setCallback( callback ); return *this; }
    Button& withFlags(int flags) { setFlags( flags); return *this; }
    Button& withChangeCallback(const std::function<void(bool)>& callback) { setChangeCallback( callback ); return *this; }
    Button& withBackgroundColor(const Color& color) { setBackgroundColor( color ); return *this; }
    Button& withIcon(int icon) { setIcon( icon ); return *this; }

    /// Set the button group (for radio buttons)
    void setButtonGroup(const std::vector<Button *> &buttonGroup) { mButtonGroup = buttonGroup; }
    const std::vector<Button *> &buttonGroup() const { return mButtonGroup; }

    virtual Vector2i preferredSize(NVGcontext *ctx) const;
    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers);
    virtual void draw(NVGcontext *ctx);
protected:
    std::string mCaption;
    int mIcon;
    IconPosition mIconPosition;
    bool mPushed;
    int mFlags;
    Color mBackgroundColor;
    Color mTextColor;
    std::function<void()> mCallback;
    std::function<void(bool)> mChangeCallback;
    std::vector<Button *> mButtonGroup;
};


class NANOGUI_EXPORT PopupButton : public Button {
public:
    PopupButton(Widget *parent, const std::string &caption = "Untitled",
                int buttonIcon = 0,
                int chevronIcon = ENTYPO_ICON_CHEVRON_SMALL_RIGHT);

    void setChevronIcon(int icon) { mChevronIcon = icon; }
    int chevronIcon() const { return mChevronIcon; }

    Popup *popup() { return mPopup; }
    const Popup* popup() const { return mPopup; }

    virtual void draw(NVGcontext* ctx);
    virtual Vector2i preferredSize(NVGcontext *ctx) const;
    virtual void performLayout(NVGcontext *ctx);

protected:
    Popup *mPopup;
    int mChevronIcon;
};

class NANOGUI_EXPORT ComboBox : public PopupButton
{
public:
    /// Create an empty combo box
    ComboBox(Widget *parent);

    /// Create a new combo box with the given items
    //ComboBox(Widget *parent, const std::vector<std::string>& items={});
    ComboBox(Widget *parent, const std::vector<std::string>& items);

    /**
     * \brief Create a new combo box with the given items, providing both short and
     * long descriptive labels for each item
     */
    ComboBox(Widget *parent, const std::vector<std::string> &items,
             const std::vector<std::string> &itemsShort);

    std::function<void(int)> callback() const { return mCallback; }
    void setCallback(const std::function<void(int)> &callback) { mCallback = callback; }

    int selectedIndex() const { return mSelectedIndex; }
    void setSelectedIndex(int idx);

    void setItems(const std::vector<std::string> &items, const std::vector<std::string> &itemsShort);
    void setItems(const std::vector<std::string> &items) { setItems(items, items); }
    const std::vector<std::string> &items() const { return mItems; }
    const std::vector<std::string> &itemsShort() const { return mItemsShort; }

    ComboBox& withItems(const std::vector<std::string>& items) {setItems(items); return *this;}
protected:
    std::vector<std::string> mItems, mItemsShort;
    std::function<void(int)> mCallback;
    int mSelectedIndex;
};

class NANOGUI_EXPORT CheckBox : public Widget
{
public:
    CheckBox(Widget *parent, const std::string &caption = "Untitled",
             const std::function<void(bool)> &callback = std::function<void(bool)>());

    const std::string &caption() const { return mCaption; }
    void setCaption(const std::string &caption) { mCaption = caption; }

    const bool &checked() const { return mChecked; }
    void setChecked(const bool &checked) { mChecked = checked; }

    CheckBox& withChecked(bool value) { setChecked(value); return *this; }

    const bool &pushed() const { return mPushed; }
    void setPushed(const bool &pushed) { mPushed = pushed; }

    std::function<void(bool)> callback() const { return mCallback; }
    void setCallback(const std::function<void(bool)> &callback) { mCallback = callback; }

    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers);
    virtual Vector2i preferredSize(NVGcontext *ctx) const;
    virtual void draw(NVGcontext *ctx);
protected:
    std::string mCaption;
    bool mPushed, mChecked;
    std::function<void(bool)> mCallback;
};

class NANOGUI_EXPORT MessageDialog : public Window {
public:
    enum class Type {
        Information,
        Question,
        Warning
    };

    MessageDialog(Widget *parent, Type type, const std::string &title = "Untitled",
                  const std::string &message = "Message",
                  const std::string &buttonText = "OK",
                  const std::string &altButtonText = "Cancel", bool altButton = false);

    Label *messageLabel() { return mMessageLabel; }
    const Label *messageLabel() const { return mMessageLabel; }

    std::function<void(int)> callback() const { return mCallback; }
    void setCallback(const std::function<void(int)> &callback) { mCallback = callback; }

    MessageDialog& withCallback(const std::function<void(int)> &callback)
    { setCallback( callback ); return *this; }
protected:
    std::function<void(int)> mCallback;
    Label *mMessageLabel;
};

class NANOGUI_EXPORT ColorWheel : public Widget
{
public:
    ColorWheel(Widget *parent, const Color& color = { 1.f, 0.f, 0.f, 1.f });

    /// Set the change callback
    std::function<void(const Color &)> callback() const                  { return mCallback;     }
    void setCallback(const std::function<void(const Color &)> &callback) { mCallback = callback; }

    /// Get the current color
    Color color() const;
    /// Set the current color
    void setColor(const Color& color);

    virtual Vector2i preferredSize(NVGcontext *ctx) const;
    virtual void draw(NVGcontext *ctx);
    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers);
    virtual bool mouseDragEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers);

private:
    enum Region {
        None = 0,
        InnerTriangle = 1,
        OuterCircle = 2,
        Both = 3
    };

    Color hue2rgb(float h) const;
    Region adjustPosition(const Vector2i &p, Region consideredRegions = Both);

protected:
    float mHue;
    float mWhite;
    float mBlack;
    Region mDragRegion;
    std::function<void(const Color &)> mCallback;
};

class NANOGUI_EXPORT ColorPicker : public PopupButton {
public:
    ColorPicker(Widget *parent, const Color& color = { 1.f, 0.f, 0.f, 1.f });

    /// Set the change callback
    std::function<void(const Color &)> callback() const                  { return mCallback; }
    void setCallback(const std::function<void(const Color &)> &callback) { mCallback = callback; }

    /// Get the current color
    Color color() const;
    /// Set the current color
    void setColor(const Color& color);

protected:
    std::function<void(const Color &)> mCallback;
    ColorWheel *mColorWheel;
    Button *mPickButton;
};

class NANOGUI_EXPORT ImageView : public Widget
{
public:
    enum class SizePolicy {
       Fixed,
       Expand
    };

    ImageView(Widget *parent, int image = 0, SizePolicy policy = SizePolicy::Fixed);

    void setImage(int img)      { mImage = img; }
    int  image() const          { return mImage; }

    void       setPolicy(SizePolicy policy) { mPolicy = policy; }
    SizePolicy policy() const { return mPolicy; }

    ImageView& withPolicy(SizePolicy policy) { setPolicy(policy); return *this; }
    ImageView& withImage(int img) { setImage(img); return *this; }

    virtual Vector2i preferredSize(NVGcontext *ctx) const;
    virtual void draw(NVGcontext* ctx);

protected:
    int mImage;
    SizePolicy mPolicy;
};

class NANOGUI_EXPORT ImagePanel : public Widget {
public:
    typedef std::vector<std::pair<int, std::string>> Images;
public:
    ImagePanel(Widget *parent);

    void setImages(const Images &data) { mImages = data; }
    const Images& images() const { return mImages; }

    std::function<void(int)> callback() const { return mCallback; }
    void setCallback(const std::function<void(int)> &callback) { mCallback = callback; }

    virtual bool mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers);
    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers);
    virtual Vector2i preferredSize(NVGcontext *ctx) const;
    virtual void draw(NVGcontext* ctx);

    ImagePanel& withImages(const Images& data ) { setImages(data); return *this; }
protected:
    Vector2i gridSize() const;
    int indexForPosition(const Vector2i &p) const;
protected:
    Images mImages;
    std::function<void(int)> mCallback;
    int mThumbSize;
    int mSpacing;
    int mMargin;
    int mMouseIndex;
};

/// Basic interface of a layout engine
class NANOGUI_EXPORT Layout : public Object {
public:
    virtual void performLayout(NVGcontext *ctx, Widget *widget) const = 0;
    virtual Vector2i preferredSize(NVGcontext *ctx, const Widget *widget) const = 0;
protected:
    virtual ~Layout() { }
};

/**
 * \brief Simple horizontal/vertical box layout
 *
 * This widget stacks up a bunch of widgets horizontally or vertically. It adds
 * margins around the entire container and a custom spacing between adjacent
 * widgets
 */
class NANOGUI_EXPORT BoxLayout : public Layout {
public:
    /**
     * \brief Construct a box layout which packs widgets in the given \c orientation
     * \param alignment
     *    Widget alignment perpendicular to the chosen orientation.
     * \param margin
     *    Margin around the layout container
     * \param spacing
     *    Extra spacing placed between widgets
     */
    BoxLayout(Orientation orientation, Alignment alignment = Alignment::Middle,
              int margin = 0, int spacing = 0);

    Orientation orientation() const { return mOrientation; }
    void setOrientation(Orientation orientation) { mOrientation = orientation; }

    Alignment alignment() const { return mAlignment; }
    void setAlignment(Alignment alignment) { mAlignment = alignment; }

    int margin() const { return mMargin; }
    void setMargin(int margin) { mMargin = margin; }

    int spacing() const { return mSpacing; }
    void setSpacing(int spacing) { mSpacing = spacing; }

    /* Implementation of the layout interface */
    Vector2i preferredSize(NVGcontext *ctx, const Widget *widget) const;
    void performLayout(NVGcontext *ctx, Widget *widget) const;

protected:
    Orientation mOrientation;
    Alignment mAlignment;
    int mMargin;
    int mSpacing;
};

/**
 * \brief Special layout for widgets grouped by labels
 *
 * This widget resembles a box layout in that it arranges a set of widgets
 * vertically. All widgets are indented on the horizontal axis except for
 * \ref Label widgets, which are not indented.
 *
 * This creates a pleasing layout where a number of widgets are grouped
 * under some high-level heading.
 */
class NANOGUI_EXPORT GroupLayout : public Layout {
public:
    GroupLayout(int margin = 15, int spacing = 6, int groupSpacing = 14,
                int groupIndent = 20)
        : mMargin(margin), mSpacing(spacing), mGroupSpacing(groupSpacing),
          mGroupIndent(groupIndent) {}

    int margin() const { return mMargin; }
    void setMargin(int margin) { mMargin = margin; }

    int spacing() const { return mSpacing; }
    void setSpacing(int spacing) { mSpacing = spacing; }

    int groupIndent() const { return mGroupIndent; }
    void setGroupIndent(int groupIndent) { mGroupIndent = groupIndent; }

    int groupSpacing() const { return mGroupSpacing; }
    void setGroupSpacing(int groupSpacing) { mGroupSpacing = groupSpacing; }

    /* Implementation of the layout interface */
    Vector2i preferredSize(NVGcontext *ctx, const Widget *widget) const;
    void performLayout(NVGcontext *ctx, Widget *widget) const;

protected:
    int mMargin;
    int mSpacing;
    int mGroupSpacing;
    int mGroupIndent;
};

/**
 * \brief Grid layout
 *
 * Widgets are arranged in a grid that has a fixed grid resolution \c resolution
 * along one of the axes. The layout orientation indicates the fixed dimension;
 * widgets are also appended on this axis. The spacing between items can be
 * specified per axis. The horizontal/vertical alignment can be specified per
 * row and column.
 */
class NANOGUI_EXPORT GridLayout : public Layout {
public:
    /// Create a 2-column grid layout by default
    GridLayout(Orientation orientation = Orientation::Horizontal, int resolution = 2,
               Alignment alignment = Alignment::Middle,
               int margin = 0, int spacing = 0)
        : mOrientation(orientation), mResolution(resolution), mMargin(margin) {
        mDefaultAlignment[0] = mDefaultAlignment[1] = alignment;
        mSpacing = Vector2i::Constant(spacing);
    }

    Orientation orientation() const { return mOrientation; }
    void setOrientation(Orientation orientation) {
        mOrientation = orientation;
    }

    int resolution() const { return mResolution; }
    void setResolution(int resolution) { mResolution = resolution; }

    int spacingX() const { return mSpacing.x(); }
    void setSpacingX(int spacing) { mSpacing.setX(spacing); }
    void setSpacing(int spacing) { mSpacing.set(spacing, spacing); }
    void setSpacing(const Vector2i& sp) { mSpacing = sp; }

    int margin() const { return mMargin; }
    void setMargin(int margin) { mMargin = margin; }

    Alignment alignment(int axis, int item) const {
        if (item < (int) mAlignment[axis].size())
            return mAlignment[axis][item];
        else
            return mDefaultAlignment[axis];
    }
    void setColAlignment(Alignment value) { mDefaultAlignment[0] = value; }
    void setRowAlignment(Alignment value) { mDefaultAlignment[1] = value; }
    void setColAlignment(const std::vector<Alignment> &value) { mAlignment[0] = value; }
    void setRowAlignment(const std::vector<Alignment> &value) { mAlignment[1] = value; }

    /* Implementation of the layout interface */
    Vector2i preferredSize(NVGcontext *ctx, const Widget *widget) const;
    void performLayout(NVGcontext *ctx, Widget *widget) const;

protected:
    // Compute the maximum row and column sizes
    void computeLayout(NVGcontext *ctx, const Widget *widget,
                       std::vector<int> *grid) const;

protected:
    Orientation mOrientation;
    Alignment mDefaultAlignment[2];
    std::vector<Alignment> mAlignment[2];
    int mResolution;
    Vector2i mSpacing;
    int mMargin;
};

/**
 * \brief Advanced Grid layout
 *
 * The is a fancier grid layout with support for items that span multiple rows
 * or columns, and per-widget alignment flags. Each row and column additionally
 * stores a stretch factor that controls how additional space is redistributed.
 * The downside of this flexibility is that a layout anchor data structure must
 * be provided for each widget.
 *
 * An example:
 *
 * <pre>
 *   using AdvancedGridLayout::Anchor;
 *   Label *label = new Label(window, "A label");
 *   // Add a centered label at grid position (1, 5), which spans two horizontal cells
 *   layout->setAnchor(label, Anchor(1, 5, 2, 1, Alignment::Middle, Alignment::Middle));
 * </pre>
 *
 * The grid is initialized with user-specified column and row size vectors
 * (which can be expanded later on if desired). If a size value of zero is
 * specified for a column or row, the size is set to the maximum preferred size
 * of any widgets contained in the same row or column. Any remaining space is
 * redistributed according to the row and column stretch factors.
 *
 * The high level usage somewhat resembles the classic HIG layout:
 * https://web.archive.org/web/20070813221705/http://www.autel.cz/dmi/tutorial.html
 * https://github.com/jaapgeurts/higlayout
 */
class NANOGUI_EXPORT AdvancedGridLayout : public Layout {
public:
    struct Anchor {
        uint8_t pos[2];
        uint8_t size[2];
        Alignment align[2];

        Anchor() { }

        Anchor(int x, int y, Alignment horiz = Alignment::Fill,
              Alignment vert = Alignment::Fill) {
            pos[0] = (uint8_t) x; pos[1] = (uint8_t) y;
            size[0] = size[1] = 1;
            align[0] = horiz; align[1] = vert;
        }

        Anchor(int x, int y, int w, int h,
              Alignment horiz = Alignment::Fill,
              Alignment vert = Alignment::Fill) {
            pos[0] = (uint8_t) x; pos[1] = (uint8_t) y;
            size[0] = (uint8_t) w; size[1] = (uint8_t) h;
            align[0] = horiz; align[1] = vert;
        }

        operator std::string() const {
            char buf[50];
            NANOGUI_SNPRINTF(buf, 50, "Format[pos=(%i, %i), size=(%i, %i), align=(%i, %i)]",
                pos[0], pos[1], size[0], size[1], (int) align[0], (int) align[1]);
            return buf;
        }
    };

    AdvancedGridLayout(const std::vector<int> &cols = {}, const std::vector<int> &rows = {});

    int margin() const { return mMargin; }
    void setMargin(int margin) { mMargin = margin; }

    /// Return the number of cols
    int colCount() const { return (int) mCols.size(); }

    /// Return the number of rows
    int rowCount() const { return (int) mRows.size(); }

    /// Append a row of the given size (and stretch factor)
    void appendRow(int size, float stretch = 0.f) { mRows.push_back(size); mRowStretch.push_back(stretch); };

    /// Append a column of the given size (and stretch factor)
    void appendCol(int size, float stretch = 0.f) { mCols.push_back(size); mColStretch.push_back(stretch); };

    /// Set the stretch factor of a given row
    void setRowStretch(int index, float stretch) { mRowStretch.at(index) = stretch; }

    /// Set the stretch factor of a given column
    void setColStretch(int index, float stretch) { mColStretch.at(index) = stretch; }

    /// Specify the anchor data structure for a given widget
    void setAnchor(const Widget *widget, const Anchor &anchor) { mAnchor[widget] = anchor; }

    /// Retrieve the anchor data structure for a given widget
    Anchor anchor(const Widget *widget) const {
        auto it = mAnchor.find(widget);
        if (it == mAnchor.end())
            throw std::runtime_error("Widget was not registered with the grid layout!");
        return it->second;
    }

    /* Implementation of the layout interface */
    Vector2i preferredSize(NVGcontext *ctx, const Widget *widget) const;
    void performLayout(NVGcontext *ctx, Widget *widget) const;

protected:
    void computeLayout(NVGcontext *ctx, const Widget *widget,
                       std::vector<int> *grid) const;

protected:
    std::vector<int> mCols, mRows;
    std::vector<float> mColStretch, mRowStretch;
    std::unordered_map<const Widget *, Anchor> mAnchor;
    int mMargin;
};

typedef std::vector<float> VectorXf;

class NANOGUI_EXPORT Graph : public Widget {
public:
    Graph(Widget *parent, const std::string &caption = "Untitled");

    const std::string &caption() const { return mCaption; }
    void setCaption(const std::string &caption) { mCaption = caption; }

    const std::string &header() const { return mHeader; }
    void setHeader(const std::string &header) { mHeader = header; }

    const std::string &footer() const { return mFooter; }
    void setFooter(const std::string &footer) { mFooter = footer; }

    const Color &backgroundColor() const { return mBackgroundColor; }
    void setBackgroundColor(const Color &backgroundColor) { mBackgroundColor = backgroundColor; }

    const Color &foregroundColor() const { return mForegroundColor; }
    void setForegroundColor(const Color &foregroundColor) { mForegroundColor = foregroundColor; }

    const Color &textColor() const { return mTextColor; }
    void setTextColor(const Color &textColor) { mTextColor = textColor; }

    const VectorXf &values() const { return mValues; }
    VectorXf &values() { return mValues; }
    void setValues(const VectorXf &values) { mValues = values; }

    virtual Vector2i preferredSize(NVGcontext *ctx) const;
    virtual void draw(NVGcontext *ctx);
protected:
    std::string mCaption, mHeader, mFooter;
    Color mBackgroundColor, mForegroundColor, mTextColor;
    VectorXf mValues;
};

class ToolButton : public Button
{
public:
    ToolButton(Widget *parent, int icon,
           const std::string &caption = "")
        : Button(parent, caption, icon)
    {
        setFlags(Flags::RadioButton | Flags::ToggleButton);
        setFixedSize(Vector2i(25, 25));
    }
};

class NANOGUI_EXPORT VScrollPanel : public Widget
{
public:
    VScrollPanel(Widget *parent);

    virtual void performLayout(NVGcontext *ctx);
    virtual Vector2i preferredSize(NVGcontext *ctx) const;
    virtual bool mouseDragEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers);
    virtual bool scrollEvent(const Vector2i &p, const Vector2f &rel);
    virtual bool mouseButtonEvent(const Vector2i &p, int button, bool down, int modifiers);
    virtual bool mouseMotionEvent(const Vector2i &p, const Vector2i &rel, int button, int modifiers);
    virtual void draw(NVGcontext *ctx);
protected:
    int mChildPreferredHeight;
    float mScroll;
};

NAMESPACE_END(nanogui)

#endif
