// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "os/OsConfigDb.h"
#include "sipXecsService/SipXecsService.h"

// DEFINES
// CONSTANTS
const char* DomainConfigurationName = "domain-config";

const char* SipXecsService::DefaultConfigurationDir = SIPX_CONFDIR;
const char* SipXecsService::DefaultLocalStateDir    = SIPX_VARDIR;
const char* SipXecsService::DefaultLogDir           = SIPX_LOGDIR;
const char* SipXecsService::DefaultRunDir           = SIPX_RUNDIR;
const char* SipXecsService::DefaultTmpDir           = SIPX_TMPDIR;
const char* SipXecsService::DefaultDatabaseDir      = SIPX_DBDIR;

DirectoryType SipXecsService::ConfigurationDirType = "SIPX_CONFDIR";
DirectoryType SipXecsService::LocalStateDirType    = "SIPX_VARDIR";
DirectoryType SipXecsService::LogDirType           = "SIPX_LOGDIR";
DirectoryType SipXecsService::RunDirType           = "SIPX_RUNDIR";
DirectoryType SipXecsService::TmpDirType           = "SIPX_TMPDIR"; 
DirectoryType SipXecsService::DatabaseDirType      = "SIPX_DBDIR";

// lookup keys for the domain configuration
const char* SipXecsService::DomainDbKey::SIP_DOMAIN_NAME = "SIP_DOMAIN_NAME";
const char* SipXecsService::DomainDbKey::SIP_REALM       = "SIP_REALM";
const char* SipXecsService::DomainDbKey::SHARED_SECRET   = "SHARED_SECRET";

// TYPEDEFS
// FORWARD DECLARATIONS

/// constructor
SipXecsService::SipXecsService(const char* serviceName)
   :mServiceName(serviceName)
{

   OsSysLog::initialize(0, mServiceName.data());

   UtlString logFileName;
   logFileName.append(mServiceName);
   logFileName.append(".log");
      
   OsPath logFilePath = Path(LogDirType, logFileName);
   OsSysLog::setOutputFile(0, logFilePath.data()) ;
   OsSysLog::enableConsoleOutput(false);
   OsSysLog::add(FAC_SIP, PRI_NOTICE, "%s >>>>>>>>>>>>>>>> STARTED",
                 mServiceName.data()
                 );

   // @TODO initialize signal handling

};


// Given a path type, return the default directory for files of that type.
const char* SipXecsService::defaultDir(DirectoryType pathType)
{
   const char* returnDir = NULL;

   if (ConfigurationDirType == pathType)
   {
      returnDir = DefaultConfigurationDir;
   }
   else if (LocalStateDirType == pathType)
   {
      returnDir = DefaultLocalStateDir;
   }
   else if (LogDirType == pathType)
   {
      returnDir = DefaultLogDir;
   }
   else if (RunDirType == pathType)
   {
      returnDir = DefaultRunDir;
   }
   else if (TmpDirType == pathType)
   {
      returnDir = DefaultTmpDir;
   }
   else if (DatabaseDirType == pathType)
   {
      returnDir = DefaultDatabaseDir;
   }
   else 
   {
      // invalid directory type
      OsSysLog::add(FAC_KERNEL, PRI_CRIT, "SipXecsService::defaultDir Invalid DirectoryType '%s'",
                    pathType);
      assert(false);
   }

   return returnDir;
}

OsPath SipXecsService::Path(DirectoryType pathType, const char* fileName) 
{
   OsPath path;

   const char* dirPath;
   if ( (dirPath = getenv(pathType)) )
   {
      OsSysLog::add(FAC_KERNEL, PRI_NOTICE,
                    "SipXecsService::Path type '%s' overridden by environment to '%s'",
                    pathType, dirPath);
   }
   else
   {
      dirPath = defaultDir(pathType);
   }
   path.append(dirPath);

   const char slash = OsPath::separator(0);
   const char lastPathChar = path(path.length()-1);
   if (fileName && *fileName != '\000')
   {
      // Add the file name
      //   make sure there is exactly one separator between the directory and the file
      if (   slash != lastPathChar
          && slash != fileName[0] 
          )
      {
         // neither has separator - add one
         path.append(OsPath::separator);
      }
      else if (   slash == lastPathChar
               && slash == fileName[0] 
               )
      {
         // both have the separator - take one off so there's only one
         path.remove(path.length()-1);
      }

      path.append(fileName);
   }
   // There is no file name, so make sure the returned directory name does not
   // end in a separator
   else if ( slash == lastPathChar )
   {
      path.remove(path.length()-1);
   } 

   OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                 "SipXecsService::Path('%s', '%s') returning '%s'",
                 pathType, fileName ? fileName : "", path.data() );
   return path;
}

/// Open the configuration common to all services in the domain.
OsPath SipXecsService::domainConfigPath()
{
   return Path(ConfigurationDirType, DomainConfigurationName);
}


/// destructor
SipXecsService::~SipXecsService()
{
   // Flush the log file
   OsSysLog::flush();
};
