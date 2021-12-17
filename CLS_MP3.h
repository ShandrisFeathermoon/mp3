#include <stdio.h>

typedef struct tagIDxHeader {
	unsigned char idx[3];
	unsigned char version;
	unsigned char revision;
	unsigned char flag;
	unsigned char size[4];
}IDxHeader;

typedef struct tagFlameHead {
	char name[4];
	unsigned char size[4];
	unsigned char flag[2];
} FlameHead;

typedef struct tagFlame {
	FlameHead head;
	int length;
	char *data;
	tagFlame *next;
	tagFlame()
		: data(NULL)
		, next(NULL)
	{
		memset(&head, 0, sizeof(FlameHead));
	}
}Flame;

class CMP3
{
public:
	CMP3(char *name);
	~CMP3();

	void ReplaceImg(const char *name);
	void Save(const char *name);
private:
	unsigned int FlameSize(const unsigned char *buf);
	void FormatFlameSize(unsigned int length, unsigned char *buf);
	Flame* GetAPIC(Flame *head);
	int FileSize(const char *name);
	void UpdateFlameLength(int length);

private:
	IDxHeader m_header;
	Flame *m_flame;
	Flame *m_tail;
	char *m_data;
	FILE *m_file;
	int m_flamelength;
	int m_flameExlength;
};

