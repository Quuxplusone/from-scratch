#include <assert.h>
#include <memory>

static bool derived_dtor_was_called = false;

struct Widget {
    virtual int foo() { return 1; }
    ~Widget() {}
};

struct WidgetImpl : public Widget, public std::enable_shared_from_this<Widget> {
    int foo() { return 2; }
    ~WidgetImpl() { derived_dtor_was_called = true; }
};

void test_crtp()
{
    my::shared_ptr<Widget> pw = my::make_shared<WidgetImpl>();
    puts("(You should have seen some output from make_shared there.)");
    assert(pw->foo() == 2);
    pw = nullptr;
    assert(derived_dtor_was_called);
}

