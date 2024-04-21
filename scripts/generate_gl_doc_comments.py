import json
import os.path
import re
import sys
from io import StringIO
from itertools import batched
from math import ceil, floor
from threading import Lock, Thread

import requests
from lxml import etree

GLAD_HEADER_PATH = os.path.join("extern", "glad", "include", "glad", "gl.h")


class GL_API_Info:
    GL_VERSION = 4

    def __init__(self, symbol, line_num):
        self.symbol: str = symbol
        self.line_num: int = line_num
        self.brief: str = None
        self.description: str = None
        self.params: list[dict[str, str]] = []

    def compare(info) -> int:
        return info.line_num

    def docs_url(self):
        return f"https://raw.githubusercontent.com/BSVino/docs.gl/mainline/gl{self.GL_VERSION}/{self.symbol}.xhtml"

    def to_json(self):
        return {
            "symbol": self.symbol,
            "line_num": self.line_num,
            "description": self.description,
            "params": self.params,
        }

    def to_doxygen_docstring(self) -> list[str]:
        if not self.description:
            return []

        ret = [
            "\n",
            "/**\n",
            f" * @brief {self.brief}\n",
            f" * @details {self.description}\n",
        ]
        for p in self.params:
            ret.append(f" * @param {p["param"]} {p["desc"]}\n")
        ret.append(" **/\n")
        return ret

    def to_xml_docstring(self) -> list[str]:
        if not self.description:
            return []

        ret = [
            f"\n",
            f"/// <summary>\n",
            f"///   {self.brief}\n",
            f"///   <para>\n",
            f"///     {self.description}\n",
            f"///   </para>\n",
            f"/// </summary>\n",
        ]
        for p in self.params:
            ret.append(f"/// <param name='{p["param"]}'>{p["desc"]}</param>\n")
        return ret


def build_symbol_map() -> list[GL_API_Info]:
    opengl_api_symbols: list[GL_API_Info] = []
    opengl_api_pattern = r"^#define (gl[A-Za-z0-9_]*?) glad_[A-Za-z0-9_]*?(?<!NV)(?<!AMD|ARB|EXT|OES|SUN|ATI|NVX)(?<!SGIX|MESA)(?<!APPLE|INTEL)$"

    with open(GLAD_HEADER_PATH, mode="r", encoding="utf-8") as header:
        for line_num, line in enumerate(header.readlines()):
            api_matches = re.match(opengl_api_pattern, line)
            if api_matches is not None:
                symbol = api_matches.groups()[0]
                info = GL_API_Info(symbol, line_num + 1)
                opengl_api_symbols.append(info)

    return opengl_api_symbols


def parse_docs_xhtml(api_list, thread_id=0, mutex=None):
    parser = etree.XMLParser()
    api_count = len(api_list)

    for api_idx, api_info in enumerate(api_list):
        with mutex:
            print(
                f"Thread:{thread_id:02} - processing {api_idx:02}/{api_count:02} ({100.0 * (api_idx/api_count):05.2f}%) => {api_info.symbol}"
            )

        docs_url: str = api_info.docs_url()
        docs_page = requests.get(docs_url)
        if docs_page.status_code != 200:
            continue

        docs_xhtml = docs_page.content.decode("utf-8")
        xhtml_root = etree.parse(StringIO(docs_xhtml), parser)

        for div in xhtml_root.xpath("//div"):
            if "class" not in div.attrib:
                continue

            if div.attrib["class"] == "refnamediv":
                api_info.brief = div.findtext("p")[len(api_info.symbol) + 3:]
            if div.attrib["class"] == "refsect1":
                if "id" in div.attrib and div.attrib["id"] == "description":
                    desc_paras = [
                        t.strip() for t in div.xpath(".//p//text()") if t.strip()
                    ]
                    api_info.description = " ".join(desc_paras)
                    while "\n" in api_info.description:
                        api_info.description = api_info.description.replace("\n", " ")
                    while "  " in api_info.description:
                        api_info.description = api_info.description.replace("  ", " ")
                    while " ." in api_info.description:
                        api_info.description = api_info.description.replace(" .", ".")

                    api_info.description = str(api_info.description)

                if "id" in div.attrib and div.attrib["id"] == "parameters":
                    params = [
                        [p.text.strip() for p in d]
                        for d in [
                            c.findall(".//dt/span/em//") for c in div.getchildren()
                        ]
                        if d and isinstance(d, list)
                    ][0]
                    for i, param_name in enumerate(params):
                        while "\n" in param_name:
                            params[i] = param_name = param_name.replace("\n", " ")
                        while "  " in param_name:
                            params[i] = param_name = param_name.replace("  ", " ")
                        while " ." in param_name:
                            params[i] = param_name = param_name.replace(" .", ".")

                    descrs = [
                        [
                            d.text.strip()
                            for d in p
                            if d is not None and d.text is not None
                        ]
                        for p in [c.findall(".//dl/dd//p") for c in div.getchildren()]
                        if p and isinstance(p, list)
                    ][0]
                    for i, param_desc in enumerate(descrs):
                        while "\n" in param_desc:
                            descrs[i] = param_desc = param_desc.replace("\n", " ")
                        while "  " in param_desc:
                            descrs[i] = param_desc = param_desc.replace("  ", " ")
                        while " ." in param_desc:
                            descrs[i] = param_desc = param_desc.replace(" .", ".")
                    api_info.params = [
                        {"param": p[0], "desc": p[1]} for p in zip(params, descrs)
                    ]


def main():
    threads = []
    lock = Lock()
    thread_count = 12

    symbols: list[dict] = build_symbol_map()
    chunk_size: int = ceil(len(symbols) / thread_count)

    for tid, chunk in enumerate(batched(symbols, chunk_size)):
        thread = Thread(
            target=parse_docs_xhtml,
            args=[chunk, tid, lock],
            name=f"Thread {id}",
        )
        threads.append(thread)
        thread.start()

    for thread in threads:
        thread.join()

    with open("./glad_apis.json", mode="w") as fh_out:
        glad_apis = json.dumps(
            [s.to_json() for s in symbols],
            sort_keys=GL_API_Info.compare,
            indent=2,
        )
        fh_out.write(glad_apis)


    with open(GLAD_HEADER_PATH, mode="r") as orig_gl_h:
        api_info = sorted(symbols, key=GL_API_Info.compare)
        with open("./gl_docstrings.h", "w", encoding="utf-8") as new_gl_h:
            api_idx = 0
            comment_line = 0

            for gl_h_linenum, gl_h_line in enumerate(orig_gl_h.readlines()):
                curr_api = api_info[api_idx] if api_idx < len(api_info) else None
                comment_line = curr_api.line_num if curr_api else sys.maxsize
                if gl_h_linenum < comment_line - 1 or not curr_api:
                    new_gl_h.write(gl_h_line)
                    continue

                curr_docstring = curr_api.to_doxygen_docstring()
                for comment_line in curr_docstring:
                    new_gl_h.write(str(comment_line))

                new_gl_h.write(gl_h_line)
                new_gl_h.flush()
                api_idx += 1


if __name__ == "__main__":
    main()
