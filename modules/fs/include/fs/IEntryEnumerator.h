#ifndef __FS_IEntryEnumerator_H__
#define __FS_IEntryEnumerator_H__

#include "defines.h"

namespace FS
{
	class IEntryEnumeratorCallBack;

	class OV_API IEntryEnumerator
	{
	public:

		class OV_API IAttributes
		{
		public:
			virtual FS::boolean isFile(void)=0;
			virtual FS::boolean isDirectory(void)=0;
			virtual FS::boolean isSymbolicLink(void)=0;

			virtual FS::boolean isArchive(void)=0;
			virtual FS::boolean isReadOnly(void)=0;
			virtual FS::boolean isHidden(void)=0;
			virtual FS::boolean isSystem(void)=0;
			virtual FS::boolean isExecutable(void)=0;

			virtual FS::uint64 getSize(void)=0;
		protected:
			virtual ~IAttributes(void);
		};

		class OV_API IEntry
		{
		public:
			virtual const char* getName(void)=0;
		protected:
			virtual ~IEntry(void);
		};

		// Returns false if nothing matched the wildcard. Possible callback errors are not currently reflected in the return value.
		virtual FS::boolean enumerate(const char* sWildcard)=0;
		virtual void release(void)=0;
	protected:
		virtual ~IEntryEnumerator(void);
	};

	class OV_API IEntryEnumeratorCallBack
	{
	public:
		virtual FS::boolean callback(FS::IEntryEnumerator::IEntry& rEntry, FS::IEntryEnumerator::IAttributes& rAttributes)=0;
		virtual ~IEntryEnumeratorCallBack(void) { }
	};

	extern OV_API FS::IEntryEnumerator* createEntryEnumerator(FS::IEntryEnumeratorCallBack& rCallBack);
};

#endif // __FS_IEntryEnumerator_H__
