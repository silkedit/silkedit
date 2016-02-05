#include "Cpp.h"

struct CppHoge::Impl {
    NSString *str; // Objective-C のオブジェクト
};


CppHoge::CppHoge()
    : pImpl_(new Impl)
{
    pImpl_->str = [[NSString alloc] init];
}


CppHoge::~CppHoge()
{
    [pImpl_->str release];
}


void CppHoge::doSomething()
{
    int length = [pImpl_->str length];
}