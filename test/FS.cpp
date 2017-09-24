#include "TestAPI.h"
#include <mgpcl/File.h>

Declare Test("io"), Priority(8.0);

TEST
{
    volatile StackIntegrityChecker sic;

    m::File wd(m::File::workingDirectory());
    m::File dot(".");

    dot.canonicalize();
    testAssert(!dot.isEmpty(), "'.' canonicalization failed!");
    testAssert(dot.path() == wd.path(), "'.' is not current directory!");

    m::File wdp(wd.parent());
    std::cout << "[i]\tWorking directory parent is: \"" << wdp.path().raw() << '\"' << std::endl;

    for(m::File &f : wdp) {
        testAssert(f.exists(), "File SHOULD exist");
        std::cout << "[i]\tFound inner file \"" << f.fileName().raw() << "\" with extension \"" << f.extension().raw() << '\"' << std::endl;
    }

    //Do it again to check for errors
    m::FileIterator it(wdp.begin());
    for(; it != wdp.end(); ++it)
        testAssert(!it.errored(), "Should'nt have errored! (looping)");

    testAssert(!it.errored(), "Should'nt have errored! (end looping)");
    return true;
}

TEST
{
    volatile StackIntegrityChecker sic;
    m::File cd(".");
    m::List<m::File> files;
    
    //std::cout << "[i]\tReal path of . is: " << cd.canonicalized().path().raw() << std::endl;

    testAssert(cd.listFilesRecursive(files), "File listing should have succeeded");

    for(m::File &f : files) {
        testAssert(f.exists(), "File SHOULD exist");
        std::cout << "[i]\tFound inner file \"" << f.path().raw() << "\" with extension \"" << f.extension().raw() << '\"' << std::endl;
    }
    return true;
}

TEST
{
    volatile StackIntegrityChecker sic;
    std::cout << "[i]\tDocuments dir: " << m::File::usualDirectory(m::kUD_UserDocuments).path().raw() << std::endl;
    std::cout << "[i]\tFont dir: " << m::File::usualDirectory(m::kUD_SystemFonts).path().raw() << std::endl;
    return true;
}
