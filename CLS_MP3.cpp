#include <sys/stat.h>
#include <string.h>

#include "CLS_MP3.h"

CMP3::CMP3(char *name)
	: m_flame(NULL)
	, m_tail(NULL)
	, m_data(NULL)
	, m_file(NULL)
	, m_flamelength(0)
	, m_flameExlength(0)
{
	memset(&m_header, 0, sizeof(IDxHeader));

	m_file = fopen(name, "rb");
	if (m_file) {
		fread(&m_header, 1, sizeof(IDxHeader), m_file);
		int n0 = (m_header.size[0] & 0x7F) << 21;
		int n1 = (m_header.size[1] & 0x7F) << 14;
		int n2 = (m_header.size[2] & 0x7F) << 7;
		int n3 = m_header.size[3] & 0x7F;
		m_flamelength = (((int)m_header.size[0] & 0x7F) << 21) +
						(((int)m_header.size[1] & 0x7F) << 14) +
						(((int)m_header.size[2] & 0x7F) << 7) +
						(int)m_header.size[3] & 0x7F;
		m_flamelength = n0 + n1 + n2 + n3;
		m_flameExlength = m_flamelength;
		while (m_flameExlength >= sizeof(FlameHead)) {
			Flame *flame = new Flame;
			fread(&flame->head, 1, sizeof(FlameHead), m_file);
			if (flame->head.name[0]) {
				flame->length = FlameSize(flame->head.size);
				flame->data = new char[flame->length];
				fread(flame->data, 1, flame->length, m_file);
				m_flameExlength -= sizeof(FlameHead) + flame->length;

				if (m_flame) {
					m_tail->next = flame;
				}
				else {
					m_flame = flame;
				}
				m_tail = flame;
			}
			else {
				//fseek(m_file, length, SEEK_CUR);
				break;
			}
		}
	}
}

CMP3::~CMP3()
{
	if (m_file) {
		fclose(m_file);
	}
	Flame *flame = m_flame;
	while (flame) {
		m_flame = flame->next;
		if (flame->data) {
			delete []flame->data;
		}
		delete flame;
		flame = m_flame;
	}
}

unsigned int CMP3::FlameSize(const unsigned char *buf)
{
	unsigned int length = 0;
	length = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + (buf[3]);
	return length;
}

void CMP3::FormatFlameSize(unsigned int length, unsigned char *buf)
{
	buf[0] = (length >> 24) & 0xFF;
	buf[1] = (length >> 16) & 0xFF;
	buf[2] = (length >> 8) & 0xFF;
	buf[3] = length & 0xFF;
}

void CMP3::ReplaceImg(const char *name)
{
	int length = FileSize(name);
	if (length > 0) {
		Flame *flame = GetAPIC(m_flame);
		if (flame) {	// update
			int offset = length - flame->length + 14;
			flame->length = length + 14;
			delete []flame->data;
			flame->data = new char[flame->length];
			memset(flame->data, 0, 14);
			strncpy(flame->data + 1, "image/jpeg", 10);
			flame->data[12] = 6;
			FormatFlameSize(flame->length, flame->head.size);
			
			FILE *file = fopen(name, "rb");
			if (file) {
				fread(flame->data + 14, 1, length, file);
			}
			UpdateFlameLength(offset);
		}
		else {			// add
			flame = new Flame;
			strncpy(flame->head.name, "APIC", 4);
			flame->length = 14 + length;
			flame->data = new char[flame->length];
			memset(flame->data, 0, 14);
			strncpy(flame->data + 1, "image/jpeg", 10);
			flame->data[12] = 6;
			FormatFlameSize(flame->length, flame->head.size);

			FILE *file = fopen(name, "rb");
			if (file) {
				fread(flame->data + 14, 1, length, file);
				if (m_flame) {
					m_tail->next = flame;
				}
				else {
					m_flame = flame;
				}
				m_tail = flame;
				UpdateFlameLength(sizeof(FlameHead) + flame->length);
			}
			else {
				printf("Failed to open %s!\n", name);
				delete []flame->data;
				delete flame;
			}
		}	
	}
	else {
		printf("File [%s] size is 0\0", name);
	}
}

void CMP3::Save(const char *name)
{
	FILE *file = fopen(name, "wb");
	if (file) {
		fwrite(&m_header, 1, sizeof(IDxHeader), file);
		Flame *flame = m_flame;
		while (flame) {
			fwrite(&flame->head, 1, sizeof(FlameHead), file);
			fwrite(flame->data, 1, flame->length, file);
			flame = flame->next;
		}

		unsigned char buf[4096] = {0};
		while (!feof(m_file)) {
			int nRead = fread(buf, 1, 4096, m_file);
			if (nRead) { 
				fwrite(buf, 1, nRead, file);
			}
		}
		fclose(file);
	}
}

void CMP3::UpdateFlameLength(int length)
{
	m_flamelength += length;
	m_header.size[0] = (m_flamelength >> 21) & 0x7F;
	m_header.size[1] = (m_flamelength >> 14) & 0x7F;
	m_header.size[2] = (m_flamelength >> 7) & 0x7F;
	m_header.size[3] = m_flamelength & 0x7F;
}

Flame* CMP3::GetAPIC(Flame *head)
{
	Flame *flame = head;
	while (flame) {
		if (strncmp(flame->head.name, "APIC", 4) == 0) {
			break;
		}
		flame = flame->next;
	}
	return flame;
}

int CMP3::FileSize(const char *name)
{
	struct stat statBuf;
	if (stat(name, &statBuf) == 0) {
		return statBuf.st_size;
	}
	return 0;
}

